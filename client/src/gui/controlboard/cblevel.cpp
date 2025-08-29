#include "cblevel.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"

extern SDLDevice *g_sdlDevice;

CBLevel::CBLevel(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        ProcessRun *argProc,
        std::function<void(Widget *)> argOnClick,

        Widget *argParent,
        bool    argAutoDelete)

    : ButtonBase
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),

          16,
          16,

          nullptr,
          nullptr,
          nullptr,
          std::move(argOnClick),

          std::nullopt,
          std::nullopt,
          std::nullopt,

          0,
          0,
          0,
          0,

          true,
          false,

          argParent,
          argAutoDelete,
      }

    , m_processRun(argProc)
    , m_image
      {
          DIR_NONE,
          [this](const Widget *){ return w() / 2; },
          [this](const Widget *){ return h() / 2; },

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
              switch(getState()){
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
          [this](const Widget *){ return w() / 2; },
          [this](const Widget *){ return h() / 2; },

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
{}
