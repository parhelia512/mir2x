#include "widget.hpp"
#include "levelbox.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "imageboard.hpp"

extern SDLDevice *g_sdlDevice;
CBLevel::CBLevel(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        ProcessRun *argProc,

        const std::function<void(int)> &argOnDrag,
        const std::function<void(   )> &argOnDoubleClick,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),

          16,
          16,

          {},

          argParent,
          argAutoDelete,
      }

    , m_processRun(argProc)
    , m_image
      {
          DIR_NONE,
          w() / 2,
          h() / 2,

          {},
          {},

          [](const Widget *)
          {
              return g_sdlDevice->getCover(8, 360);
          },

          false,
          false,
          0,

          [this](const Widget *) -> uint32_t
          {
              switch(m_state){
                  case BEVENT_ON  : return colorf::BLUE + colorf::A_SHF(0XFF);
                  case BEVENT_DOWN: return colorf::RED  + colorf::A_SHF(0XFF);
                  default         : return 0;
              }
          },

          this,
          false,
      }

    , m_level
      {
          DIR_NONE,
          w() / 2,
          h() / 2,

          [this](const Widget *) -> std::string
          {
              return std::to_string(m_processRun->getMyHero()->getLevel());
          },

          0,
          12,
          0,
          colorf::YELLOW + colorf::A_SHF(255),

          this,
          false,
      }

    , m_onDrag(argOnDrag)
    , m_onDoubleClick(argOnDoubleClick)
{}

bool CBLevel::processEventDefault(const SDL_Event &event, bool valid, int startDstX, int startDstY)
{
    if(!valid){
        return false;
    }

    switch(event.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                if(!in(event.button.x, event.button.y, startDstX, startDstY)){
                    m_state = BEVENT_OFF;
                    return false;
                }

                if(event.button.clicks == 2){
                    m_onDoubleClick();
                }
                m_state = BEVENT_DOWN;
                return true;
            }
        case SDL_MOUSEBUTTONUP:
            {
                if(!in(event.button.x, event.button.y, startDstX, startDstY)){
                    m_state = BEVENT_OFF;
                    return false;
                }

                m_state = BEVENT_ON;
                return true;
            }
        case SDL_MOUSEMOTION:
            {
                // even not in the box
                // we still need to drag the widget

                if(m_state == BEVENT_DOWN){
                    if(event.motion.state & SDL_BUTTON_LMASK){
                        m_onDrag(event.motion.yrel);
                        return true;
                    }
                    else{
                        if(in(event.motion.x, event.motion.y, startDstX, startDstY)){
                            m_state = BEVENT_ON;
                            return true;
                        }
                        else{
                            m_state = BEVENT_OFF;
                            return false;
                        }
                    }
                }

                if(in(event.motion.x, event.motion.y, startDstX, startDstY)){
                    m_state = BEVENT_ON;
                    return true;
                }
                else{
                    m_state = BEVENT_OFF;
                    return false;
                }
            }
        default:
            {
                return false;
            }
    }
}
