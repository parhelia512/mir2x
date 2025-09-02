#pragma once
#include "widget.hpp"
#include "gfxcropdupboard.hpp"
#include "npcchatorigframe.hpp"

class NPCChatFrame: public Widget
{
    private:
        NPCChatOrigFrame m_frame;

    private:
        GfxCropDupBoard m_board;

    public:
        NPCChatFrame(
                Widget::VarDir,
                Widget::VarInt,
                Widget::VarInt,

                Widget::VarSize,
                Widget::VarSize,

                Widget * = nullptr,
                bool     = false);
};
