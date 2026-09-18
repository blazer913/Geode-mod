#pragma once
#include <Geode/Geode.hpp>
#include <vector>
#include <algorithm>

using namespace geode::prelude;

// Stores exactly what happened, when it happened, and to which player
struct MacroInput {
    int frame;
    PlayerButton button;
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

    static MacroEngine& get() {
        static MacroEngine instance;
        return instance;
    }

    void toggleRecording() {
        if (m_state == State::Recording) {
            m_state = State::Idle;
            log::info("Recording stopped. Total inputs: {}", m_inputs.size());
        } else {
            m_state = State::Recording;
            m_inputs.clear();
            m_currentFrame = 0;
            log::info("Recording started.");
        }
    }

    void togglePlayback() {
        if (m_state == State::Playing) {
            m_state = State::Idle;
            log::info("Playback stopped.");
        } else {
            if (m_inputs.empty()) {
                log::warn("No macro recorded to play!");
                return;
            }
            m_state = State::Playing;
            m_playbackIndex = 0;
            m_currentFrame = 0;
            log::info("Playback started.");
        }
    }

    void recordInput(PlayerButton button, bool isDown, bool isPlayer1) {
        if (m_state == State::Recording) {
            m_inputs.push_back({m_currentFrame, button, isDown, isPlayer1});
        }
    }
    
    // The Practice Mode Fix: Rewinds the timeline on death
    void syncOnReset() {
        if (m_state == State::Recording) {
            // Erase any inputs that occurred after the frame we just respawned at
            m_inputs.erase(
                std::remove_if(m_inputs.begin(), m_inputs.end(),
                    [this](const MacroInput& input) { return input.frame >= this->m_currentFrame; }),
                m_inputs.end()
            );
        } else if (m_state == State::Playing) {
            // Rewind the playback index to match the respawn frame
            m_playbackIndex = 0;
            while (m_playbackIndex < m_inputs.size() && m_inputs[m_playbackIndex].frame < m_currentFrame) {
                m_playbackIndex++;
            }
        }
    }
};
