#pragma once
#include <functional>

namespace cbot {
enum class Command {
    RecordToggle, PlayPause, FrameBack, FrameForward, OverlayToggle
};

class Controller {
public:
    using Callback = std::function<void(Command)>;
    void setCallback(Callback cb);
    void command(Command c);
private:
    Callback m_cb;
};
}
