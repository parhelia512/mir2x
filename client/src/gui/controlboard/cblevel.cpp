#include "cblevel.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"

extern SDLDevice *g_sdlDevice;

CBLevel::CBLevel(
        Widget::VarDir argDir,
        Widget::VarInt argX,
        Widget::VarInt argY,

        ProcessRun *argProc,
        std::function<void(Widget *, int)> argOnClick,

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

          SDL_BLENDMODE_NONE,

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
              if(auto myHero = m_processRun->getMyHero(true)){
                  return std::to_string(myHero->getLevel());
              }
              else{
                  return "?";
              }
          },

          0,
          12,
          0,

          colorf::YELLOW + colorf::A_SHF(255),
          SDL_BLENDMODE_NONE,

          this,
          false,
      }
{}
