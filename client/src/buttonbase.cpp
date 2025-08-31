#include <functional>
#include "sdldevice.hpp"
#include "buttonbase.hpp"
#include "sdldevice.hpp"
#include "soundeffectdb.hpp"

extern SDLDevice *g_sdlDevice;
extern SoundEffectDB *g_seffDB;

ButtonBase::ButtonBase(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        Widget::VarSize argW,
        Widget::VarSize argH,

        std::function<void(Widget *           )> argOnOverIn,
        std::function<void(Widget *           )> argOnOverOut,
        std::function<void(Widget *, bool, int)> argOnClick,
        std::function<void(Widget *,       int)> argOnTrigger,

        std::optional<uint32_t> argSeffIDOnOverIn,
        std::optional<uint32_t> argSeffIDOnOverOut,
        std::optional<uint32_t> argSeffIDOnClick,

        int argOffXOnOver,
        int argOffYOnOver,
        int argOffXOnClick,
        int argOffYOnClick,

        bool argOnClickDone,
        bool argRadioMode,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),
          std::move(argW),
          std::move(argH),

          {},

          argParent,
          argAutoDelete,
      }

    , m_onClickDone(argOnClickDone)
    , m_radioMode(argRadioMode)

    , m_seffID
      {
          argSeffIDOnOverIn,
          argSeffIDOnOverOut,
          argSeffIDOnClick,
      }

    , m_offset
      {
          {0               , 0             },
          {argOffXOnOver   , argOffYOnOver },
          {argOffXOnClick  , argOffYOnClick},
      }

    , m_onOverIn (std::move(argOnOverIn ))
    , m_onOverOut(std::move(argOnOverOut))
    , m_onClick  (std::move(argOnClick  ))
    , m_onTrigger(std::move(argOnTrigger))
{}

bool ButtonBase::processEventDefault(const SDL_Event &event, bool valid, int startDstX, int startDstY, const Widget::ROIOpt &roi)
{
    if(!valid){
        if(m_radioMode){
            if(getState() == BEVENT_ON){
                setState(BEVENT_OFF);
                onOverOut();
            }
        }
        else{
            if(getState() != BEVENT_OFF){
                setState(BEVENT_OFF);
                onOverOut();
            }
        }
        return consumeFocus(false);
    }

    if(!active()){
        if(m_radioMode){
            if(getState() == BEVENT_ON){
                setState(BEVENT_OFF);
            }
        }
        else{
            if(getState() != BEVENT_OFF){
                setState(BEVENT_OFF);
            }
        }
        return consumeFocus(false);
    }

    switch(event.type){
        case SDL_MOUSEBUTTONUP:
            {
                if(in(event.button.x, event.button.y, startDstX, startDstY, roi)){
                    switch(getState()){
                        case BEVENT_OFF:
                            {
                                setState(BEVENT_ON);
                                onBadEvent();
                                break;
                            }
                        case BEVENT_DOWN:
                            {
                                if(m_radioMode){
                                    // keep pressed
                                }
                                else{
                                    setState(BEVENT_ON);
                                    onClick(true, event.button.clicks);
                                    if(m_onClickDone){
                                        onTrigger(event.button.clicks);
                                    }
                                }
                                break;
                            }
                        default:
                            {
                                break;
                            }
                    }
                    return consumeFocus(true);
                }
                else if(m_radioMode){
                    return consumeFocus(false);
                }
                else{
                    if(getState() != BEVENT_OFF){
                        setState(BEVENT_OFF);
                        onOverOut();
                    }
                    return consumeFocus(false);
                }
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                if(in(event.button.x, event.button.y, startDstX, startDstY, roi)){
                    switch(getState()){
                        case BEVENT_OFF:
                            {
                                setState(BEVENT_DOWN);
                                onBadEvent();
                                break;
                            }
                        case BEVENT_ON:
                            {
                                setState(BEVENT_DOWN);
                                onClick(false, event.button.clicks);
                                if(!m_onClickDone){
                                    onTrigger(event.button.clicks);
                                }
                                break;
                            }
                        default:
                            {
                                break;
                            }
                    }
                    return consumeFocus(true);
                }
                else if(m_radioMode){
                    return consumeFocus(false);
                }
                else{
                    if(getState() != BEVENT_OFF){
                        setState(BEVENT_OFF);
                        onOverOut();
                    }
                    return consumeFocus(false);
                }
            }
        case SDL_MOUSEMOTION:
            {
                if(in(event.motion.x, event.motion.y, startDstX, startDstY, roi)){
                    switch(getState()){
                        case BEVENT_OFF:
                            {
                                setState(BEVENT_ON);
                                onOverIn();
                                break;
                            }
                        case BEVENT_DOWN:
                            {
                                if(event.motion.state & SDL_BUTTON_LMASK){
                                    // hold the button and moving
                                    // don't trigger
                                }
                                else if(m_radioMode){
                                    // keep pressed
                                }
                                else{
                                    setState(BEVENT_ON);
                                    onBadEvent();
                                }
                                break;
                            }
                        default:
                            {
                                break;
                            }
                    }
                    return consumeFocus(true);
                }
                else if(m_radioMode){
                    if(getState() == BEVENT_ON){
                        setState(BEVENT_OFF);
                        onOverOut();
                    }
                    return consumeFocus(false);
                }
                else{
                    if(getState() != BEVENT_OFF){
                        setState(BEVENT_OFF);
                        onOverOut();
                    }
                    return consumeFocus(false);
                }
            }
        default:
            {
                return consumeFocus(false);
            }
    }
}

void ButtonBase::onOverIn()
{
    if(m_onOverIn){
        m_onOverIn(this);
    }

    if(m_seffID[0].has_value()){
        g_sdlDevice->playSoundEffect(g_seffDB->retrieve((m_seffID[0].value())));
    }
}

void ButtonBase::onOverOut()
{
    if(m_onOverOut){
        m_onOverOut(this);
    }

    if(m_seffID[1].has_value()){
        g_sdlDevice->playSoundEffect(g_seffDB->retrieve((m_seffID[1].value())));
    }
}

void ButtonBase::onClick(bool clickDone, int clickCount)
{
    if(m_onClick){
        m_onClick(this, clickDone, clickCount);
    }

    if(clickDone){
        // press button
    }
    else{
        if(m_seffID[2].has_value()){
            g_sdlDevice->playSoundEffect(g_seffDB->retrieve((m_seffID[2].value())));
        }
    }
}

void ButtonBase::onTrigger(int clickCount)
{
    if(m_onTrigger){
        m_onTrigger(this, clickCount);
    }
}

void ButtonBase::onBadEvent()
{
}
