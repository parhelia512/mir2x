#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <functional>

#include "log.hpp"
#include "colorf.hpp"
#include "totype.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "imageboard.hpp"
#include "processrun.hpp"
#include "controlboard.hpp"
#include "clientmonster.hpp"
#include "teamstateboard.hpp"

extern Log *g_log;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

CBMiddle::CBMiddle(
        Widget::VarDir argDir,
        Widget::VarInt argX,
        Widget::VarInt argY,

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
              g_sdlDevice->fillRectangle(colorf::A_SHF(0XFF), drawDstX, drawDstY, self->w(), self->h());
          },

          .parent{this},
      }}

    , m_face
      {
          DIR_UPLEFT,
          0,
          0,
          86,
          96,

          argProc,

          this,
          false,
      }

    , m_bgImgFull
      {{
          .texLoadFunc = [](const Widget *)
          {
              return g_progUseDB->retrieve(0X00000013);
          },
      }}

    , m_bgImg
      {
          DIR_UPLEFT,
          0,
          0,

          [this](const Widget *){ return             w(); },
          [this](const Widget *){ return m_bgImgFull.h(); },

          &m_bgImgFull,

          50,
          0,
          287,
          [this](const Widget *){ return m_bgImgFull.h(); },

          this,
          false,
      }

    , m_switchMode
      {{
          .x = [this](const Widget *){ return w() - 17; },
          .y = 3,

          .texIDList
          {
              .on   = 0X00000028,
              .down = 0X00000029,
          },

          .onTrigger = [this](Widget *, int clickCount)
          {
              if(auto parptr = hasParent<ControlBoard>()){
                  parptr->onClickSwitchModeButton(clickCount);
              }
          },

          .parent{this},
      }}

    , m_slider
      {{
          .bar
          {
              .x = [this]{ return w() - 178; },
              .y = 40,
              .w = 5,
              .h = 60,
              .v = true,
          },

          .index = 2,
          .parent{this},
      }}

    , m_logView
      {
          DIR_UPLEFT,
          9,
          11,

          std::addressof(m_logBoard),

          0,
          [this](const Widget *) { return std::max<int>(0, to_dround((m_logBoard.h() - 83) * m_slider.getValue())); },
          [this](const Widget *) { return m_logBoard.w(); },
          83,

          {},

          this,
          false,
      }
{
    setH([this]{ return m_bgImgFull.h(); });
}

bool CBMiddle::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
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
