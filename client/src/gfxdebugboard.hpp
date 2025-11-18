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
        ShapeCropBoard m_imgFrame;
        MarginWrapper  m_imgContainer;

        SliderBase m_imgResizeHSlider;
        SliderBase m_imgResizeVSlider;

        SliderBase m_cropHSlider_0;
        SliderBase m_cropHSlider_1;

        SliderBase m_cropVSlider_0;
        SliderBase m_cropVSlider_1;

    public:
        GfxDebugBoard(GfxDebugBoard::InitArgs);
};
