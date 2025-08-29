#pragma once
#include <functional>
#include "widget.hpp"
#include "bevent.hpp"
#include "textboard.hpp"
#include "imageboard.hpp"

class ProcessRun;
class CBLevel: public ButtonBase
{
    private:
        ProcessRun *m_processRun;

    private:
        ImageBoard m_image;
        TextBoard  m_level;

    public:
        CBLevel(
                Widget::VarDir,
                Widget::VarOff,
                Widget::VarOff,

                ProcessRun *,
                std::function<void(Widget *)>,

                Widget * = nullptr,
                bool     = false);
};
