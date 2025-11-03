#pragma once
#include <cstdint>
#include <SDL2/SDL.h>
#include "totype.hpp"
#include "widget.hpp"
#include "sliderbase.hpp"
#include "imageboard.hpp"
#include "shapecropboard.hpp"

class TexSlider: public Widget
{
    protected:
        using BarArgs = SliderBase::BarArgs;

    private:
        struct InitArgs final
        {
            BarArgs bar {};

            int index = 0;

            Widget::VarUpdateFunc<float> onChange = nullptr;
            Widget::WADPair parent {};
        };

    private:
        struct SliderTexInfo
        {
            const int w;
            const int h;

            // define the texture center
            // some slider textures are not in good shape then not using the (w / 2, h / 2)

            const int offX;
            const int offY;

            const int cover;
            const uint32_t texID;
        };

        constexpr static SliderTexInfo m_sliderTexInfoList []
        {
            { 8,  8,  7,  8, 5, 0X00000080},
            {18, 18,  9,  9, 5, 0X00000081},
            { 5,  5,  8,  9, 4, 0X00000088},
            { 8,  8, 12, 13, 7, 0X00000089},
            { 8,  8, 12, 13, 7, 0X0000008A},
        };

    private:
        const SliderTexInfo *m_sliderTexInfo;

    private:
        ImageBoard m_image;
        ImageBoard m_cover;

    private:
        SliderBase m_base;

    private:
        ShapeCropBoard m_debugDraw;

    public:
        TexSlider(TexSlider::InitArgs);

    public:
        bool vbar() const
        {
            return m_base.vbar();
        }

    public:
        float getValue() const
        {
            return m_base.getValue();
        }

    public:
        void setValue(float value, bool triggerCallback){ m_base.setValue(value, triggerCallback); }
        void addValue(float value, bool triggerCallback){ m_base.addValue(value, triggerCallback); }
};
