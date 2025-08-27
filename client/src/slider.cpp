#include <SDL2/SDL.h>
#include "slider.hpp"

std::tuple<int, int> Slider::getValueCenter(int startDstX, int startDstY) const
{
    return
    {
        startDstX + std::lround(( m_hslider ? getValue() : 0.5f) * w()),
        startDstY + std::lround((!m_hslider ? getValue() : 0.5f) * h()),
    };
}

std::tuple<int, int, int, int> getSliderRectangle(int startDstX, int startDstY) const
{
    const auto [centerX, centerY] = getValueCenter();
    return
    {
        centerX - m_sliderW / 2,
        centerY - m_sliderH / 2,
        m_sliderW,
        m_sliderH,
    };
}

bool Slider::processEventDefault(const SDL_Event &event, bool valid, int startDstX, int startDstY)
{
    if(!valid){
        return consumeFocus(false);
    }

    if(!active()){
        return consumeFocus(false);
    }

    const auto fnInSlider = [this](int eventX, int eventY) -> bool
    {
        const auto [sliderX, sliderY, sliderW, sliderH] = getSliderRectangle();
        return mathf::pointInRectangle<int>(eventX, eventY, sliderX, sliderY, sliderW, sliderH);
    };

    switch(event.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                if(fnInSlider(event.button.x, event.button.y)){
                    m_sliderState = BEVENT_DOWN;
                    return consumeFocus(true);
                }
                else if(in(event.button.x, event.button.y)){
                    m_sliderState = BEVENT_ON;
                    setValue([&event, this]() -> float
                    {
                        if(m_hslider){
                            return ((event.button.x - x()) * 1.0f) / std::max<int>(1, w());
                        }
                        else{
                            return ((event.button.y - y()) * 1.0f) / std::max<int>(1, h());
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
                if(fnInSlider(event.button.x, event.button.y)){
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
                    if(fnInSlider(event.motion.x, event.motion.y) || focus()){
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
                    if(fnInSlider(event.motion.x, event.motion.y)){
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
