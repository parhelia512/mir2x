#include "colorf.hpp"
#include "sdldevice.hpp"
#include "gfxdebugboard.hpp"

extern SDLDevice *g_sdlDevice;
GfxDebugBoard::GfxDebugBoard(GfxDebugBoard::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),
          .w = 400,
          .h = 300,

          .parent = std::move(args.parent),
      }}

    , m_bg
      {{
          . w = [this]{ return w(); },
          . h = [this]{ return h(); },

          .drawFunc = [this](int dstDrawX, int dstDrawY)
          {
              g_sdlDevice->fillRectangle(colorf::BLACK + colorf::A_SHF(0X40), dstDrawX, dstDrawY, w(), h());
              g_sdlDevice->drawRectangle(colorf::WHITE + colorf::A_SHF(0X80), dstDrawX, dstDrawY, w(), h());
          },
      }}
{}
