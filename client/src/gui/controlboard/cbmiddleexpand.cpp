#include "sdldevice.hpp"
#include "controlboard.hpp"
#include "cbmiddleexpand.hpp"

extern SDLDevice *g_sdlDevice;

CBMiddleExpand::CBMiddleExpand(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        Widget::VarSize argW,
        Widget::VarSize argH,

        ProcessRun *argProc,

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

    , m_processRun(argProc)
    , m_logBoard(hasParent<ControlBoard>()->m_logBoard)

    , m_bg
      {
          DIR_UPLEFT,
          0,
          0,
          [this](const Widget *){ return w(); },
          [this](const Widget *){ return h(); },

          [this](const Widget *self, int drawDstX, int drawDstY)
          {
              g_sdlDevice->fillRectangle(colorf::A_SHF(0XF0), drawDstX, drawDstY, self->w(), self->h());
          },

          this,
          false,
      }

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

    , m_bgImgFull
      {
          DIR_UPLEFT,
          0,
          0,

          {},
          {},

          [](const Widget *)
          {
              return g_progUseDB->retrieve(0X00000027);
          },
      }

    , m_bgImg
      {
          DIR_UPLEFT,
          0,
          0,

          [this](const Widget *){ return w(); }
          [this](const Widget *){ return h(); }

          &m_bgImgFull,

          50,
          47,
          287,
          196,
          {},

          this,
          false,
      }

    , m_buttonSwitchMode
      {
          DIR_UPLEFT,
          boardW - 178 - 181,
          3,
          {SYS_U32NIL, 0X00000028, 0X00000029},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *)
          {
              switchExpandMode();
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          this,
          false,
      }

    , m_buttonEmoji
      {
          DIR_UPLEFT,
          boardW - 178 - 260,
          87,
          {SYS_U32NIL, 0X00000023, 0X00000024},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          nullptr,

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          this,
          false,
      }

    , m_buttonMute
      {
          DIR_UPLEFT,
          boardW - 178 - 220,
          87,
          {SYS_U32NIL, 0X00000025, 0X00000026},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          nullptr,

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          this,
          false,
      }

    , m_slider
      {
          DIR_UPLEFT,
          boardW - 178 - 176,
          40,
          5,
          60,

          false,
          2,
          nullptr,

          this,
          false,
      }
{}
