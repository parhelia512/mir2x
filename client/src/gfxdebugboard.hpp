#pragma once
#include "widget.hpp"
#include "imageboard.hpp"
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

    private:
        ImageBoard m_img;

    private:
        ShapeCropBoard m_imgBg;

    public:
        GfxDebugBoard(GfxDebugBoard::InitArgs);
};
