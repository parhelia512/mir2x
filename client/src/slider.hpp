//       +---+
// +-----|   |----------------+
// |     |   |                |
// +-----|   |----------------+
//       +---+    ^
//         ^      |
//         |      +-------------- chute
//         +--------------------- slider

#pragma once
#include <SDL2/SDL.h>
#include <functional>
#include "widget.hpp"
#include "bevent.hpp"

class Slider: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarOff x = 0;
            Widget::VarOff y = 0;

            Widget::VarSizeOpt w = 0;
            Widget::VarSizeOpt h = 0;

            bool hslider = true;

            int sliderW = 10;
            int sliderH = 10;

            float value = 0.0f;

            Widget::VarUpdateFunc<float> onChange = nullptr;
            Widget::WADPair parent {};
        };

    private:
        const bool m_hslider;
        const int  m_sliderW;
        const int  m_sliderH;

    private:
        float m_value;

    protected:
        int m_sliderState = BEVENT_OFF;

    private:
        const Widget::VarUpdateFunc<float> m_onChange;

    public:
        Slider(Slider::InitArgs);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        bool hslider() const
        {
            return m_hslider;
        }

        float getValue() const
        {
            return m_value;
        }

    public:
        virtual void setValue(float, bool);
        virtual void addValue(float, bool) final;

    protected:
        float pixel2Value(int) const;

    public:
        std::tuple<int, int> getValueCenter(int, int) const;
        std::tuple<int, int, int, int> getSliderRectangle(int, int) const;

    protected:
        bool inSlider(int, int, Widget::ROIMap) const;
};
