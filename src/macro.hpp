#pragma once
#include <Geode/Geode.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

using namespace geode::prelude;

struct MacroInput {
    int frame;
    int button; 
    bool isDown;
    bool isPlayer1;
};

class MacroEngine {
public:
    enum class State { Idle, Recording, Playing };
    
    State m_state = State::Idle;
    std::vector<MacroInput> m_inputs;
    
    size_t m_playbackIndex = 0;
    int m_currentFrame = 0;
    bool m_isPlaybackInput = false;

    static MacroEngine& get() {
        static MacroEngine instance;
        return instance;
    }

    void recordInput(int button, bool isDown, bool isPlayer1) {
        if (m_state != State::Recording || m_isPlaybackInput) return;

        if (!m_inputs.empty()) {
            auto& last = m_inputs.back();
            if (last.frame == m_currentFrame && last.button == button &&
                last.isDown == isDown && last.isPlayer1 == isPlayer1) {
                return; // likely a duplicate from a second hook catching the same press
            }
        }

        m_inputs.push_back({m_currentFrame, button, isDown, isPlayer1});
    }
    
    void saveCurrentMacroToFile() {
        if (m_inputs.empty()) return;
        
        auto saveDir = Mod::get()->getSaveDir();
        std::filesystem::create_directories(saveDir);
        
        int count = 1;
        auto path = saveDir / ("Macro_" + std::to_string(count) + ".txt");
        
        while (std::filesystem::exists(path)) {
            count++;
            path = saveDir / ("Macro_" + std::to_string(count) + ".txt");
        }
        
        std::ofstream file(path);
        if (file.is_open()) {
            for (const auto& input : m_inputs) {
                file << input.frame << "," << input.button << "," 
                     << input.isDown << "," << input.isPlayer1 << "\n";
            }
            file.close();
            log::info("Macro saved to: {}", path.string());
        }
    }
    
    bool loadMacroFromFile(const std::string& filename) {
        auto path = Mod::get()->getSaveDir() / filename;
        std::ifstream file(path);
        
        if (!file.is_open()) return false;
        
        m_inputs.clear();
        std::string line;
        
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string item;
            MacroInput input;
            
            try {
                std::getline(ss, item, ','); input.frame = std::stoi(item);
                std::getline(ss, item, ','); input.button = std::stoi(item);
                std::getline(ss, item, ','); input.isDown = std::stoi(item) != 0;
                std::getline(ss, item, ','); input.isPlayer1 = std::stoi(item) != 0;
                m_inputs.push_back(input);
            } catch (...) {
                log::error("Corrupted line in macro file: {}", line);
                continue;
            }
        }
        
        file.close();
        return true;
    }

    std::vector<std::string> getSavedMacroFiles() {
        std::vector<std::string> files;
        auto saveDir = Mod::get()->getSaveDir();
        
        if (std::filesystem::exists(saveDir)) {
            for (const auto& entry : std::filesystem::directory_iterator(saveDir)) {
                if (entry.path().extension() == ".txt") {
                    files.push_back(entry.path().filename().string());
                }
            }
        }
        return files;
    }
    
    void syncOnReset() {
        if (m_state == State::Recording) {
            m_inputs.erase(
                std::remove_if(m_inputs.begin(), m_inputs.end(),
                    [this](const MacroInput& input) { return input.frame >= this->m_currentFrame; }),
                m_inputs.end()
            );
        } else if (m_state == State::Playing) {
            m_playbackIndex = 0;
            while (m_playbackIndex < m_inputs.size() && m_inputs[m_playbackIndex].frame < m_currentFrame) {
                m_playbackIndex++;
            }
        }
    }
};
