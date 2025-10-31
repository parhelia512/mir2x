#include <SDL2/SDL.h>
#include "slider.hpp"

Slider::Slider(Slider::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),
          .w = std::move(args.w),
          .h = std::move(args.h),

          .parent = std::move(args.parent),
      }}

    , m_hslider(args.hslider)
    , m_sliderW(args.sliderW)
    , m_sliderH(args.sliderH)

    , m_value(fflcheck(args.value, args.value >= 0.0f && args.value <= 1.0f))
    , m_onChange(std::move(args.onChange))
{}

bool Slider::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return consumeFocus(false);
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(!active()){
        return consumeFocus(false);
    }

    switch(event.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                if(inSlider(event.button.x, event.button.y, m)){
                    m_sliderState = BEVENT_DOWN;
                    return consumeFocus(true);
                }
                else if(m.in(event.button.x, event.button.y)){
                    m_sliderState = BEVENT_ON;
                    setValue([&event, startDstX = m.x, startDstY = m.y, roiOpt = m.ro, this]() -> float
                    {
                        if(m_hslider){
                            return ((event.button.x - (startDstX - roiOpt->x)) * 1.0f) / std::max<int>(1, w());
                        }
                        else{
                            return ((event.button.y - (startDstY - roiOpt->y)) * 1.0f) / std::max<int>(1, h());
                        }
                    }(), true);
                    return consumeFocus(true);
                }
                else{
                    m_sliderState = BEVENT_OFF;
                    return consumeFocus(false);
                }
            }
        case SDL_MOUSEBUTTONUP:
            {
                if(inSlider(event.button.x, event.button.y, m)){
                    m_sliderState = BEVENT_ON;
                    return consumeFocus(true);
                }
                else{
                    m_sliderState = BEVENT_OFF;
                    return consumeFocus(false);
                }
            }
        case SDL_MOUSEMOTION:
            {
                if(event.motion.state & SDL_BUTTON_LMASK){
                    if(inSlider(event.motion.x, event.motion.y, m) || focus()){
                        m_sliderState = BEVENT_DOWN;
                        if(m_hslider){
                            addValue(pixel2Value(event.motion.xrel), true);
                        }
                        else{
                            addValue(pixel2Value(event.motion.yrel), true);
                        }
                        return consumeFocus(true);
                    }
                    else{
                        m_sliderState = BEVENT_OFF;
                        return consumeFocus(false);
                    }
                }
                else{
                    if(inSlider(event.motion.x, event.motion.y, m)){
                        m_sliderState = BEVENT_ON;
                        return consumeFocus(true);
                    }
                    else{
                        m_sliderState = BEVENT_OFF;
                        return consumeFocus(false);
                    }
                }
            }
        default:
            {
                return consumeFocus(false);
            }
    }
}

void Slider::setValue(float value, bool triggerCallback)
{
    if(const auto newValue = std::clamp<float>(value, 0.0f, 1.0f); newValue != getValue()){
        m_value = newValue;
        if(triggerCallback && Widget::hasUpdateFunc(m_onChange)){
            Widget::execUpdateFunc(m_onChange, this, getValue());
        }
    }
}

void Slider::addValue(float diff, bool triggerCallback)
{
    setValue(m_value + diff, triggerCallback);
}

float Slider::pixel2Value(int pixel) const
{
    return pixel * 1.0f / std::max<int>(m_hslider ? w() : h(), 1);
}

std::tuple<int, int> Slider::getValueCenter(int startDstX, int startDstY) const
{
    return
    {
        startDstX + to_dround(( m_hslider ? getValue() : 0.5f) * w()),
        startDstY + to_dround((!m_hslider ? getValue() : 0.5f) * h()),
    };
}

std::tuple<int, int, int, int> Slider::getSliderRectangle(int startDstX, int startDstY) const
{
    const auto [centerX, centerY] = getValueCenter(startDstX, startDstY);
    return
    {
        centerX - m_sliderW / 2,
        centerY - m_sliderH / 2,
        m_sliderW,
        m_sliderH,
    };
}

bool Slider::inSlider(int eventX, int eventY, Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return false;
    }

    if(auto sliderROI = Widget::makeROI(getSliderRectangle(m.x - m.ro->x, m.y - m.ro->y)); sliderROI.crop({m.x, m.y, m.ro->w, m.ro->h})){
        return sliderROI.in(eventX, eventY);
    }
    return false;
}
