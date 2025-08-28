#pragma once
#include "widget.hpp"
#include "cbleft.hpp"
#include "cbright.hpp"
#include "cbmiddle.hpp"
#include "cbmiddleexpand.hpp"
#include "cbtitle.hpp"

class ProcessRun;
class ControlBoard: public Widget
{
    private:
        enum CBMode
        {
            MODE_DEFAULT,

            MODE_HIDE,
            MODE_EXPAND,
            MODE_MAXIMIZE,
        };

    private:
        ProcessRun *m_processRun;

    private:
        CBMode m_mode = MODE_DEFAULT;

    private:
        CBLeft  m_left;
        CBRight m_right;

        CBTitle m_title;

        CBMiddle       m_middle;
        CBMiddleExpand m_middleExpand;
};
