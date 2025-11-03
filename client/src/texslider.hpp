#pragma once
#include <cstdint>
#include <SDL2/SDL.h>
#include "totype.hpp"
#include "widget.hpp"
#include "sliderbase.hpp"
#include "imageboard.hpp"

class TexSlider: public SliderBase
{
    protected:
        using SliderBase::BarArgs;

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
        struct SliderTexInfo
        {
            // define the texture center
            // some slider textures are not in good shape then not using the (w / 2, h / 2)

            const int offX;
            const int offY;

            const int cover;
            const uint32_t texID;
        };

        constexpr static SliderTexInfo m_sliderTexInfoList []
        {
            { 7,  7, 5, 0X00000080},
            { 9,  8, 5, 0X00000081},
            { 7,  8, 4, 0X00000088},
            {10, 12, 7, 0X00000089},
            {14, 15, 7, 0X0000008A},
        };

    private:
        static constexpr auto getSliderTexInfo(int index)
        {
            fflassert(index >= 0);
            fflassert(index < static_cast<int>(std::size(m_sliderTexInfoList)));
            return m_sliderTexInfoList + index;
        }

    public:
        TexSlider(TexSlider::InitArgs);
};
