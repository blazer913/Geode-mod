#pragma once
#include <Geode/Geode.hpp>
#include <vector>
#include <map>
#include <string>
#include <algorithm>

using namespace geode::prelude;

struct MacroInput {
    int frame;
    int button; // Stored as int to easily cast back to PlayerButton
    bool isDown;
    bool isPlayer1;
};

class MacroEngine {
public:
    enum class State { Idle, Recording, Playing };
    
    State m_state = State::Idle;
    std::vector<MacroInput> m_inputs; // The active macro
    
    // Storage for all saved macros
    std::map<std::string, std::vector<MacroInput>> m_savedMacros;
    
    size_t m_playbackIndex = 0;
    int m_currentFrame = 0;

    static MacroEngine& get() {
        static MacroEngine instance;
        return instance;
    }

    void recordInput(int button, bool isDown, bool isPlayer1) {
        if (m_state == State::Recording) {
            m_inputs.push_back({m_currentFrame, button, isDown, isPlayer1});
        }
    }
    
    void saveCurrentMacro() {
        if (m_inputs.empty()) return;
        
        // Auto-generate a name (e.g., "Macro 1", "Macro 2")
        std::string macroName = "Macro " + std::to_string(m_savedMacros.size() + 1);
        m_savedMacros[macroName] = m_inputs;
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
