#pragma once
#include <Geode/Geode.hpp>
#include <vector>
#include <algorithm>

using namespace geode::prelude;

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
    
    // Flag to prevent the game from recording the bot's own simulated clicks
    bool m_isPlaybackInput = false; 

    static MacroEngine& get() {
        static MacroEngine instance;
        return instance;
    }

    void recordInput(PlayerButton button, bool isDown, bool isPlayer1) {
        if (m_state == State::Recording) {
            m_inputs.push_back({m_currentFrame, button, isDown, isPlayer1});
        }
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
