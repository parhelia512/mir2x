#pragma once
#include <functional>
#include "widget.hpp"
#include "bevent.hpp"
#include "sdldevice.hpp"
#include "textboard.hpp"
#include "imageboard.hpp"

class ProcessRun;
class LevelBox: public Widget
{
    private:
        ProcessRun *m_processRun;

    private:
        ImageBoard m_image;
        TextBoard  m_level;

    private:
        int m_state = BEVENT_OFF;

    private:
        std::function<void(int)> m_onDrag;
        std::function<void(   )> m_onDoubleClick;

    public:
        LevelBox(
                Widget::VarDir,
                Widget::VarOff,
                Widget::VarOff,

                ProcessRun *,

                const std::function<void(int)> &, // drag
                const std::function<void(   )> &, // double-click

                Widget * = nullptr, // parent
                bool     = false);  // auto-delete

    public:
        bool processEventDefault(const SDL_Event &, bool, int, int) override;
};
