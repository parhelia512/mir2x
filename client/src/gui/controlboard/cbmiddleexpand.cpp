#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "controlboard.hpp"
#include "cbmiddleexpand.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

CBMiddleExpand::CBMiddleExpand(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        Widget::VarSizeOpt argW,

        ProcessRun *argProc,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),
          .w = std::move(argW),

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_processRun(argProc)
    , m_logBoard(hasParent<ControlBoard>()->m_logBoard)

    , m_bg
      {{
          .w = [this]{ return w(); },
          .h = [this]{ return h(); },

          .drawFunc = [this](const Widget *self, int drawDstX, int drawDstY)
          {
              g_sdlDevice->fillRectangle(colorf::A_SHF(0XF0), drawDstX, drawDstY, self->w(), self->h());
          },

          .parent{this},
      }}

    , m_logView
      {
          DIR_UPLEFT,
          9,
          11,

          std::addressof(m_logBoard),

          0,
          [this]{ return std::max<int>(0, to_dround((m_logBoard.h() - 83) * m_slider.getValue())); },
          [this]{ return m_logBoard.w(); },
          83,

          {},

          this,
          false,
      }

    , m_bgImgFull
      {{
          .texLoadFunc = [](const Widget *){ return g_progUseDB->retrieve(0X00000027); },
      }}

    , m_bgImg
      {
          DIR_UPLEFT,
          0,
          0,

          [this]{ return w(); },
          [this]{ return h(); },

          &m_bgImgFull,

          50,
          47,
          287,
          196,

          this,
          false,
      }

    , m_buttonSwitchMode
      {{
          .x = [this]{ return w() - 178; },
          .y = 3,

          .texIDList
          {
              .on   = 0X00000028,
              .down = 0X00000029,
          },

          .onTrigger = [this](Widget *, int clickDone)
          {
              hasParent<ControlBoard>()->onClickSwitchModeButton(clickDone);
          },

          .parent{this},
      }}

    , m_buttonEmoji
      {{
          .x = [this]{ return w() - 178; },
          .y = 87,

          .texIDList
          {
              .on   = 0X00000023,
              .down = 0X00000024,
          },

          .parent{this},
      }}

    , m_buttonMute
      {{
          .x = [this]{ return w() - 220; },
          .y = 87,

          .texIDList
          {
              .on   = 0X00000025,
              .down = 0X00000026,
          },

          .parent{this},
      }}

    , m_slider
      {{
          .x = [this]{ return w() - 176; },
          .y = 40,
          .w = 5,
          .h = 60,

          .hslider = false,
          .sliderIndex = 2,

          .parent{this},
      }}
{
    setH([this]
    {
        switch(hasParent<ControlBoard>()->m_mode){
            case CBM_EXPAND:
                {
                    return std::min<int>(400, g_sdlDevice->getRendererHeight());
                }
            case CBM_MAXIMIZE:
                {
                    return g_sdlDevice->getRendererHeight();
                }
            default:
                {
                    return 0;
                }
        }
    });
}

bool CBMiddleExpand::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(Widget::processEventDefault(event, valid, m)){
        return true;
    }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym){
                    case SDLK_RETURN:
                        {
                            return valid && hasParent<ControlBoard>()->m_cmdLine.consumeFocus(true);
                        }
                    default:
                        {
                            return false;
                        }
                }
            }
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEMOTION:
        default:
            {
                return false;
            }
    }
}
