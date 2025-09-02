#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "npcchatorigframe.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

NPCChatOrigFrame::NPCChatOrigFrame(
        Widget::VarDir argDir,
        Widget::VarInt argX,
        Widget::VarInt argY,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),

          {},
          {},
          {},

          argParent,
          argAutoDelete,
      }

    , m_up
      {
          DIR_UPLEFT,
          0,
          0,

          {},
          {},

          [](const Widget *) -> SDL_Texture *
          {
              return g_progUseDB->retrieve(0X00000051);
          },

          false,
          false,
          0,

          colorf::WHITE_A255,
          SDL_BLENDMODE_NONE,

          this,
          false,
      }

    , m_down
      {
          DIR_UPLEFT,
          0,
          [this](const Widget *){ return m_up.h(); },

          {},
          {},

          [](const Widget *) -> SDL_Texture *
          {
              return g_progUseDB->retrieve(0X00000053);
          },

          false,
          false,
          0,

          colorf::WHITE_A255,
          SDL_BLENDMODE_NONE,

          this,
          false,
      }
{}
