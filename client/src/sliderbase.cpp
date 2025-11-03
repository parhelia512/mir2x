#include <SDL2/SDL.h>
#include "sdldevice.hpp"
#include "sliderbase.hpp"
#include "clientargparser.hpp"

extern SDLDevice *g_sdlDevice;
extern ClientArgParser *g_clientArgParser;

SliderBase::SliderBase(SliderBase::InitArgs args)
    : Widget
      {{
          .parent = std::move(args.parent),
      }}

    , m_barArgs(std::move(args.bar))
    , m_sliderArgs(std::move(args.slider))

    , m_value(fflcheck(args.value, args.value >= 0.0f && args.value <= 1.0f))
    , m_onChange(std::move(args.onChange))

    , m_bar
      {{
          .dir = std::move(args.bar.dir),

          .x = [this]{ return -1 * widgetXFromBar(0); },
          .y = [this]{ return -1 * widgetYFromBar(0); },

          .w = [this]{ return Widget::evalSize(m_barArgs.w, this); },
          .h = [this]{ return Widget::evalSize(m_barArgs.w, this); },

          .contained = std::move(args.barWidget),
          .parent{this},
      }}

    , m_slider
      {{
          .x = [this]{ return sliderXAtValue(m_bar.dx(), getValue()); },
          .y = [this]{ return sliderYAtValue(m_bar.dy(), getValue()); },

          .w = [this]{ return Widget::evalSize(m_sliderArgs.w, this); },
          .h = [this]{ return Widget::evalSize(m_sliderArgs.h, this); },

          .contained = std::move(args.sliderWidget),
          .parent{this},
      }}

    , m_debugDraw
      {{
          .w = [this]{ return w(); },
          .h = [this]{ return h(); },

          .drawFunc = [this](const Widget *self, int drawDstX, int drawDstY)
          {
              if(g_clientArgParser->debugSlider){
                  g_sdlDevice->drawRectangle(colorf::GREEN_A255, drawDstX, drawDstY, self->w(), self->h());

                  const auto [valCenterX, valCenterY] = getValueCenter(drawDstX, drawDstY);
                  g_sdlDevice->drawLine(colorf::YELLOW_A255, drawDstX, drawDstY, valCenterX, valCenterY);

                  const auto r = getSliderROI(drawDstX, drawDstY);
                  g_sdlDevice->drawRectangle(colorf::RED_A255, r.x, r.y, r.w, r.h);
              }
          },

          .parent{this},
      }}
{
    moveTo([this]{ return widgetXFromBar(Widget::evalInt(m_barArgs.x, this))); },
           [this]{ return widgetYFromBar(Widget::evalInt(m_barArgs.y, this))); });

    setSize([this]{ return std::max<int>(m_bar.dx() + m_bar.w(), sliderXAtValue(1.0f) + m_slider.w()) - dx(); },
            [this]{ return std::max<int>(m_bar.dy() + m_bar.h(), sliderYAtValue(1.0f) + m_slider.h()) - dy(); });
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
        startDstX + m_slider.dx() + Widget::evalInt(m_sliderArgs.cx.value_or(m_slider.w() / 2), this),
        startDstY + m_slider.dy() + Widget::evalInt(m_sliderArgs.cy.value_or(m_slider.h() / 2), this),
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

int SliderBase::sliderXAtValue(int barX, float value) const
{
    fflassert(value >= 0.0f, value);
    fflassert(value <= 1.0f, value);

    const auto barW = Widget::evalSize(m_barArgs.w, this);
    const auto sliderW = Widget::evalSize(m_sliderArgs.w, this);
    const auto sliderCX = Widget::evalInt(m_sliderArgs.cx.value_or(sliderW / 2), this);

    return vbar()
        ? (barX - sliderCX + barW / 2)
        : (barX - sliderCX + to_dround(value * (barW - 1)));
}

int SliderBase::sliderYAtValue(int barY, float value) const
{
    fflassert(value >= 0.0f, value);
    fflassert(value <= 1.0f, value);

    const auto barH = Widget::evalSize(m_barArgs.h, this);
    const auto sliderH = Widget::evalSize(m_sliderArgs.h, this);
    const auto sliderCY = Widget::evalInt(m_sliderArgs.cy.value_or(sliderH / 2), this);

    return vbar()
        ? (barY - sliderCY + to_dround(value * (barH - 1)))
        : (barY - sliderCY + sliderH / 2);
}
