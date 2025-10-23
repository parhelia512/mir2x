#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "npcchatorigframe.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

NPCChatOrigFrame::NPCChatOrigFrame(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),
          .w = {},
          .h = {},

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_up
      {{
          .texLoadFunc = [] -> SDL_Texture *
          {
              return g_progUseDB->retrieve(0X00000051);
          },

          .blendMode = SDL_BLENDMODE_NONE,
          .parent{this},
      }}

    , m_down
      {{
          .x = 0,
          .y = [this]{ return m_up.h(); },

          .texLoadFunc = [] -> SDL_Texture *
          {
              return g_progUseDB->retrieve(0X00000053);
          },

          .blendMode = SDL_BLENDMODE_NONE,
          .parent{this},
      }}
{}
