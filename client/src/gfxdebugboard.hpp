#pragma once
#include "widget.hpp"
#include "itemflex.hpp"
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
        Widget          m_srcWidget;
        Widget                  m_imgCanvas;

        MarginWrapper                   m_imgContainer;
        ShapeCropBoard                  m_imgFrame;

        SliderBase              m_imgResizeHSlider;
        SliderBase              m_imgResizeVSlider;

        SliderBase              m_cropHSlider_0;
        SliderBase              m_cropHSlider_1;

        SliderBase              m_cropVSlider_0;
        SliderBase              m_cropVSlider_1;

        TextBoard               m_imgSize;
        TextBoard               m_roiInfo;

        GfxResizeBoard  m_dstBoard;

    public:
        GfxDebugBoard(GfxDebugBoard::InitArgs);

    public:
        Widget::ROI getROI() const; // take img's DIR_UPLEFT as (0, 0)
};
