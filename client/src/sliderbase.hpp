// +----------------------------- InitArgs::{dir, x, y}
// |
// v     +---+
// *-----|   |----------------+
// |     |   |                |
// +-----|   |----------------+
//       +---+    ^
//         ^      |
//         |      +-------------- bar
//         +--------------------- slider

#pragma once
#include <SDL2/SDL.h>
#include <functional>
#include "widget.hpp"
#include "bevent.hpp"
#include "margincontainer.hpp"

class SliderBase: public Widget
{
    public:
        struct BarArgs final
        {
            // full widget's location is decided by bar position and size
            // slider position and size are relative to bar

            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarOff x = 0;
            Widget::VarOff y = 0;

            Widget::VarSizeOpt w = 0;
            Widget::VarSizeOpt h = 0;

            bool v = true; // vertical bar
        };

        struct SliderArgs final
        {
            // value center of slider may not be at the geometric center
            // can happen when using tex as slider

            // (cx, cy) overlaps with bar geometric center when slider value is 0.5, in pixel-level

            Widget::VarOffOpt cx = std::nullopt;
            Widget::VarOffOpt cy = std::nullopt;

            Widget::VarSize w = 10;
            Widget::VarSize h = 10;
        };

    private:
        struct InitArgs final
        {
            BarArgs bar {};
            SliderArgs slider {};

            float value = 0.0f;

            MarginContainer::ContainedWidget    barWidget {};
            MarginContainer::ContainedWidget sliderWidget {};

            Widget::VarUpdateFunc<float> onChange = nullptr;
            Widget::WADPair parent {};
        };

    private:
        const BarArgs m_barArgs;
        const SliderArgs m_sliderArgs;

    private:
        float m_value;

    private:
        int m_sliderState = BEVENT_OFF;

    private:
        const Widget::VarUpdateFunc<float> m_onChange;

    private:
        MarginContainer m_bar;
        MarginContainer m_slider;

    public:
        SliderBase(SliderBase::InitArgs);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        int sliderState() const
        {
            return m_sliderState;
        }

    public:
        bool vbar() const
        {
            return m_barArgs.v;
        }

        float getValue() const
        {
            return m_value;
        }

    public:
        void setValue(float, bool);
        void addValue(float, bool);

    protected:
        float pixel2Value(int) const;

    public:
        Widget::ROI getSliderROI(int, int) const;
        std::tuple<int, int> getValueCenter(int, int) const;

    protected:
        bool inSlider(int, int, Widget::ROIMap) const;

    private:
        int sliderXAtValue(int, float) const; // only depends on barArgs and sliderArgs
        int sliderYAtValue(int, float) const; // ....

    private:
        int widgetXFromBar(int barX) const { return std::min<int>(barX, sliderXAtValue(barX, 0.0f)); }
        int widgetYFromBar(int barY) const { return std::min<int>(barY, sliderXAtValue(barY, 0.0f)); }
};
