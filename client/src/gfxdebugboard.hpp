#pragma once
#include "widget.hpp"
#include "sliderbase.hpp"
#include "imageboard.hpp"
#include "marginwrapper.hpp"
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
        Widget         m_imgWidget;
        MarginWrapper  m_imgContainer;
        ShapeCropBoard m_imgFrame;

        SliderBase m_imgResizeHBar;
        SliderBase m_imgResizeVBar;

    public:
        GfxDebugBoard(GfxDebugBoard::InitArgs);
};
