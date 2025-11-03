#include <SDL2/SDL.h>
#include "sliderbase.hpp"

SliderBase::SliderBase(SliderBase::InitArgs args)
    : Widget
      {{
          .parent = std::move(args.parent),
      }}

    , m_vbar(args.bar.v)
    , m_sliderArgs(std::move(args.slider))

    , m_value(fflcheck(args.value, args.value >= 0.0f && args.value <= 1.0f))
    , m_onChange(std::move(args.onChange))

    , m_bar
      {{
          .dir = std::move(args.bar.dir),

          .w = std::move(args.bar.w),
          .h = std::move(args.bar.h),

          .parent{this},
      }}

    , m_slider
      {{
          .w = [this]{ return Widget::evalSize(m_sliderArgs.w, this); },
          .h = [this]{ return Widget::evalSize(m_sliderArgs.h, this); },

          .parent{this},
      }}
{
    m_bar.moveTo([this]{ return -1 * std::get<0>(widgetPosFromBar(0, 0)); },
                 [this]{ return -1 * std::get<1>(widgetPosFromBar(0, 0)); });

    m_slider.moveTo([this]{ return std::get<0>(sliderPosAtValue(m_bar.dx()),          0, getValue())); },
                    [this]{ return std::get<1>(sliderPosAtValue(0          , m_bar.dy(), getValue())); });

    moveTo([barX = std::move(args.bar.x), this]{ return std::get<0>(widgetPosFromBar(Widget::evalOff(barX, this),                           0)); },
           [barY = std::move(args.bar.y), this]{ return std::get<1>(widgetPosFromBar(0                          , Widget::evalOff(barY, this))); });

    setSize([this]{ return std::max<int>(m_bar.dx() + m_bar.w(), std::get<0>(sliderPosAtValue(1.0f) + m_slider.w())) - dx(); },
            [this]{ return std::max<int>(m_bar.dy() + m_bar.h(), std::get<1>(sliderPosAtValue(1.0f) + m_slider.h())) - dy(); });
}

bool SliderBase::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
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

void SliderBase::setValue(float value, bool triggerCallback)
{
    if(const auto newValue = std::clamp<float>(value, 0.0f, 1.0f); newValue != getValue()){
        m_value = newValue;
        if(triggerCallback && Widget::hasUpdateFunc(m_onChange)){
            Widget::execUpdateFunc(m_onChange, this, getValue());
        }
    }
}

void SliderBase::addValue(float diff, bool triggerCallback)
{
    setValue(m_value + diff, triggerCallback);
}

float SliderBase::pixel2Value(int pixel) const
{
    return pixel * 1.0f / std::max<int>(vbar() ? (m_bar.h() - 1) : (m_bar.w() - 1), 1);
}

Widget::ROI SliderBase::getSliderROI(int startDstX, int startDstY) const
{
    return Widget::ROI
    {
        .x = startDstX + m_slider.dx(),
        .y = startDstY + m_slider.dy(),
        .w =             m_slider. w(),
        .h =             m_slider. h(),
    };
}

std::tuple<int, int> SliderBase::getValueCenter(int startDstX, int startDstY) const
{
    return
    {
        startDstX + m_slider.dx() + Widget::evalOff(m_sliderArgs.cx.value_or(m_slider.w() / 2), this),
        startDstY + m_slider.dy() + Widget::evalOff(m_sliderArgs.cy.value_or(m_slider.h() / 2), this),
    };
}

bool SliderBase::inSlider(int eventX, int eventY, Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return false;
    }

    if(auto sliderROI = getSliderROI(); sliderROI.crop({m.x, m.y, m.ro->w, m.ro->h})){
        return sliderROI.in(eventX, eventY);
    }
    return false;
}

int SliderBase::sliderXAtValue(int barX, float value) const // depends on m_bar.w
{
    fflassert(value >= 0.0f, value);
    fflassert(value <= 1.0f, value);

    const auto sliderW = Widget::evalSize(m_sliderArgs.w, this); // don't use m_slider.w(), initialization may not be done yet
    const auto sliderCX = Widget::evalOff(m_sliderArgs.cx.value_or(sliderW / 2), this);

    return vbar() ? (barX - (sliderCX - m_bar.w() / 2)) : (barX + to_dround(value * (m_bar.w() - 1)) - sliderCX);
}

std::tuple<int, int> SliderBase::sliderPosAtValue(std::optinal<int> barX, std::optional<int> barY, float value) const // depends on m_bar.w/h
{
    fflassert(value >= 0.0f, value);
    fflassert(value <= 1.0f, value);

    const auto sliderW  = Widget::evalSize(m_sliderArgs.w, this); // don't use m_slider.w(), initialization may not be done yet
    const auto sliderH  = Widget::evalSize(m_sliderArgs.h, this);

    const auto sliderCX = Widget::evalOff(m_sliderArgs.cx.value_or(sliderW / 2), this);
    const auto sliderCY = Widget::evalOff(m_sliderArgs.cy.value_or(sliderH / 2), this);

    if(vbar()){
        return
        {
            barX - (sliderCX - sliderW / 2),
            barY + to_dround(value * (m_bar.h() - 1)) - sliderCY,
        };
    }
    else{
        return
        {
            barX + to_dround(value * (m_bar.w() - 1)) - sliderCX,
            barY - (sliderCY - sliderH / 2),
        };
    }
}

std::tuple<int, int> SliderBase::widgetPosFromBar(int barX, int barY) const
{
    const auto [sliderX, sliderY] = sliderPosAtValue(barX, barY, 0.0f);
    return
    {
        std::min<int>(barX, sliderX),
        std::min<int>(barY, sliderY),
    };
}
