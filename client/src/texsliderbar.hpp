#pragma once
#include <cstdint>
#include "widget.hpp"
#include "texslider.hpp"
#include "imageboard.hpp"
#include "gfxdupboard.hpp"
#include "gfxresizeboard.hpp"

// slider uses slot: 0X00000460
//              bar: 0X00000470
//
// slot:
//
//   |<-3->|             v
//   +-----------------  -
//   |                   2
//   |     +-----------  -  -
//   |     |             ^  ^
//   |     |                |
//   |     |                5
//   |     |                |
//   |     |                v  v
//   |     +-----------     -  -
//   |                         2
//   +-----------------        -
//                             ^

class TexSliderBar: public TexSlider
{
    public:
        constexpr static int SLOT_FIXED_EDGE_SIZE = 9;
        constexpr static int  BAR_FIXED_EDGE_SIZE = 5;

    protected:
        using TexSlider::BarArgs;
        using TexSlider::BarBgWidget;

    private:
        struct InitArgs final
        {
            BarArgs bar {};

            int index = 0;
            float value = 0.0f;

            Widget::VarUpdateFunc<float> onChange = nullptr;
            Widget::WADPair parent {};
        };

    private:
        ImageBoard m_imgSlot; // pixels are as above
        ImageBoard m_imgBar;  // 5 pixels height, horizontal direction is identical

        GfxResizeBoard m_slot;
        GfxDupBoard    m_bar;

    public:
        TexSliderBar(TexSliderBar::InitArgs);
};
