#pragma once
#include <functional>
#include "widget.hpp"
#include "bevent.hpp"
#include "textboard.hpp"
#include "imageboard.hpp"
#include "buttonbase.hpp"

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
                Widget::VarInt,
                Widget::VarInt,

                ProcessRun *,
                std::function<void(Widget *, int)>,

                Widget * = nullptr,
                bool     = false);
};
