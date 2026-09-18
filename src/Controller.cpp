#include "Controller.hpp"
namespace cbot {
void Controller::setCallback(Callback cb) { m_cb=std::move(cb); }
void Controller::command(Command c) { if (m_cb) m_cb(c); }
}
