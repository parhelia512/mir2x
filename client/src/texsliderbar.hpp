#pragma once
#include <cstdint>
#include "widget.hpp"
#include "texslider.hpp"
#include "imageboard.hpp"
#include "gfxdupboard.hpp"
#include "gfxresizeboard.hpp"

class TexSliderBar: public TexSlider
{
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
        ImageBoard m_imgSlot;
        ImageBoard m_imgBar;

        GfxResizeBoard m_slot;
        GfxDupBoard    m_bar;

    public:
        TexSliderBar(TexSliderBar::InitArgs);
};
