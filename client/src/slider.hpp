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
#include "mathf.hpp"
#include "widget.hpp"
#include "bevent.hpp"

class Slider: public Widget
{
    private:
        const bool m_hslider = true;

    protected:
        const int m_sliderW = 0;
        const int m_sliderH = 0;

    protected:
        int m_sliderState = BEVENT_OFF;

    private:
        float m_value = 0.0f;

    private:
        const std::function<void(float)> m_onChanged;

    public:
        Slider(
                Widget::VarDir argDir,
                Widget::VarOff argX,
                Widget::VarOff argY,

                Widget::VarSizeOpt argW,
                Widget::VarSizeOpt argH,

                bool argHSlider,

                int argSliderW,
                int argSliderH,

                std::function<void(float)> argOnChanged,

                Widget *argParent     = nullptr,
                bool    argAutoDelete = false)

            : Widget
              ({
                  .dir = std::move(argDir),

                  .x = std::move(argX),
                  .y = std::move(argY),
                  .w = std::move(argW),
                  .h = std::move(argH),

                  .parent
                  {
                      .widget = argParent,
                      .autoDelete = argAutoDelete,
                  }
              })

            , m_hslider(argHSlider)
            , m_sliderW(argSliderW)
            , m_sliderH(argSliderH)
            , m_onChanged(std::move(argOnChanged))
        {}

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
            if(const auto newValue = mathf::bound<float>(value, 0.0f, 1.0f); newValue != getValue()){
                m_value = newValue;
                if(triggerCallback && m_onChanged){
                    m_onChanged(getValue());
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
