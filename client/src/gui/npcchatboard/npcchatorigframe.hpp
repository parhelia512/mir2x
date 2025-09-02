#pragma once
#include "widget.hpp"
#include "imageboard.hpp"

class NPCChatOrigFrame: public Widget
{
    private:
        ImageBoard m_up;
        ImageBoard m_down;

    public:
        NPCChatOrigFrame(
                Widget::VarDir,
                Widget::VarInt,
                Widget::VarInt,

                Widget * = nullptr,
                bool     = false);
};
