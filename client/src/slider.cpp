#include <SDL2/SDL.h>
#include "slider.hpp"

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

bool Slider::processEventDefault(const SDL_Event &event, bool valid, int startDstX, int startDstY, const Widget::ROIOpt &roi)
{
    const auto roiOpt = cropDrawROI(startDstX, startDstY, roi);
    if(!roiOpt.has_value()){
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
                if(inSlider(event.button.x, event.button.y, startDstX, startDstY, roiOpt.value())){
                    m_sliderState = BEVENT_DOWN;
                    return consumeFocus(true);
                }
                else if(in(event.button.x, event.button.y, startDstX, startDstY, roiOpt.value())){
                    m_sliderState = BEVENT_ON;
                    setValue([&event, startDstX, startDstY, &roiOpt, this]() -> float
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
                if(inSlider(event.button.x, event.button.y, startDstX, startDstY, roiOpt.value())){
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
                    if(inSlider(event.motion.x, event.motion.y, startDstX, startDstY, roiOpt.value()) || focus()){
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
                    if(inSlider(event.motion.x, event.motion.y, startDstX, startDstY, roiOpt.value())){
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

bool Slider::inSlider(int eventX, int eventY, int startDstX, int startDstY, const Widget::ROIOpt &roi) const
{
    const auto roiOpt = cropDrawROI(startDstX, startDstY, roi);
    if(!roiOpt.has_value()){
        return false;
    }

    const auto [sliderX, sliderY, sliderW, sliderH] = getSliderRectangle(startDstX - roiOpt->x, startDstY - roiOpt->y);
    Widget::ROI sliderROI
    {
        .x = sliderX,
        .y = sliderY,
        .w = sliderW,
        .h = sliderH,
    };

    sliderROI = Widget::ROI(startDstX, startDstY, roiOpt->w, roiOpt->h).create(sliderROI);

    if(sliderROI.empty()){
        return false;
    }

    return sliderROI.in(eventX, eventY);
}
