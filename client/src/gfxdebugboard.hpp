#pragma once
#include "widget.hpp"
#include "shapecropboard.hpp"
#include "gfxresizeboard.hpp"

class GfxDebugBoard: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::WADPair parent {};
        };

    private:
        ShapeCropBoard m_bg;

    public:
        GfxDebugBoard(GfxDebugBoard::InitArgs);
};
