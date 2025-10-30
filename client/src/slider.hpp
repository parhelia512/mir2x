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
        float getValue() const
        {
            return m_value;
        }

    public:
        virtual void setValue(float value, bool triggerCallback)
        {
            if(const auto newValue = std::clamp<float>(value, 0.0f, 1.0f); newValue != getValue()){
                m_value = newValue;
                if(triggerCallback && Widget::hasUpdateFunc(m_onChange)){
                    Widget::execUpdateFunc(m_onChange, this, getValue());
                }
            }
        }

    public:
        void addValue(float diff, bool triggerCallback)
        {
            setValue(m_value + diff, triggerCallback);
        }

    protected:
        float pixel2Value(int pixel) const
        {
            return pixel * 1.0f / std::max<int>(m_hslider ? w() : h(), 1);
        }

    public:
        std::tuple<int, int> getValueCenter(int, int) const;
        std::tuple<int, int, int, int> getSliderRectangle(int, int) const;

    public:
        bool hslider() const
        {
            return m_hslider;
        }

    protected:
        bool inSlider(int, int, Widget::ROIMap) const;
};
