#include "sdldevice.hpp"
#include "pngtexdb.hpp"
#include "horseboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

HorseBoard::HorseBoard(
        dir8_t argDir,

        int argX,
        int argY,

        ProcessRun *argProc,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          argDir,
          argX,
          argY,

          257,
          322,

          {},

          argParent,
          argAutoDelete,
      }

    , m_processRun(argProc)
    , m_greyBg
      {
          DIR_UPLEFT,
          0,
          0,

          [this](const Widget *){ return w(); },
          [this](const Widget *){ return h(); },

          [](const Widget *self, int drawDstX, int drawDstY)
          {
              g_sdlDevice->fillRectangle(colorf::GREY + colorf::A_SHF(255), drawDstX, drawDstY, self->w(), self->h());
          },

          this,
          false,
      }

    , m_imageBg
      {
          DIR_UPLEFT,
          0,
          0,

          {},
          {},

          [](const Widget *)
          {
              return g_progUseDB->retrieve(0X00000700);
          },

          false,
          false,
          0,

          colorf::WHITE_A255,
          SDL_BLENDMODE_NONE,

          this,
          false,
      }

    , m_close
      {
          DIR_UPLEFT,
          217,
          278,
          {SYS_U32NIL, 0X0000001C, 0X0000001D},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int)
          {
              setShow(false);
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          this,
      }

    , m_up
      {
          DIR_UPLEFT,
          20,
          231,
          {SYS_U32NIL, 0X00000710, 0X00000711},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int)
          {
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          this,
      }

    , m_down
      {
          DIR_UPLEFT,
          77,
          231,
          {SYS_U32NIL, 0X00000720, 0X00000721},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int)
          {
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          this,
      }

    , m_hide
      {
          DIR_UPLEFT,
          134,
          231,
          {SYS_U32NIL, 0X00000730, 0X00000731},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int)
          {
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          this,
      }

    , m_show
      {
          DIR_UPLEFT,
          191,
          231,
          {SYS_U32NIL, 0X00000740, 0X00000741},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int)
          {
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          this,
      }
{
    setShow(false);
}

bool HorseBoard::processEventDefault(const SDL_Event &event, bool valid, int startDstX, int startDstY, const Widget::ROIOpt &roi)
{
    const auto roiOpt = cropDrawROI(startDstX, startDstY, roi);
    if(!roiOpt.has_value()){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(m_close.processParentEvent(event, valid, startDstX, startDstY, roiOpt.value())){ return true; }
    if(m_up   .processParentEvent(event, valid, startDstX, startDstY, roiOpt.value())){ return true; }
    if(m_down .processParentEvent(event, valid, startDstX, startDstY, roiOpt.value())){ return true; }
    if(m_hide .processParentEvent(event, valid, startDstX, startDstY, roiOpt.value())){ return true; }
    if(m_show .processParentEvent(event, valid, startDstX, startDstY, roiOpt.value())){ return true; }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym){
                    case SDLK_ESCAPE:
                        {
                            setShow(false);
                            setFocus(false);
                            return true;
                        }
                    default:
                        {
                            return consumeFocus(false);
                        }
                }
            }
        case SDL_MOUSEMOTION:
            {
                if((event.motion.state & SDL_BUTTON_LMASK) && (in(event.motion.x, event.motion.y, startDstX, startDstY, roiOpt.value()) || focus())){
                    const auto remapXDiff = startDstX - roiOpt->x;
                    const auto remapYDiff = startDstY - roiOpt->y;

                    const auto [rendererW, rendererH] = g_sdlDevice->getRendererSize();
                    const int maxX = rendererW - w();
                    const int maxY = rendererH - h();

                    const int newX = std::max<int>(0, std::min<int>(maxX, remapXDiff + event.motion.xrel));
                    const int newY = std::max<int>(0, std::min<int>(maxY, remapYDiff + event.motion.yrel));

                    moveBy(newX - remapXDiff, newY - remapYDiff);
                    return consumeFocus(true);
                }
                return consumeFocus(false);
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                return consumeFocus(true);
            }
        default:
            {
                return consumeFocus(false);
            }
    }
}
