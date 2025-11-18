#include "colorf.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "gfxdebugboard.hpp"

extern PNGTexDB *g_progUseDB;
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
          .w = [this]{ return w(); },
          .h = [this]{ return h(); },

          .drawFunc = [this](int dstDrawX, int dstDrawY)
          {
              g_sdlDevice->fillRectangle(colorf::BLACK + colorf::A_SHF(0XF0), dstDrawX, dstDrawY, w(), h());
              g_sdlDevice->drawRectangle(colorf::WHITE + colorf::A_SHF(0X80), dstDrawX, dstDrawY, w(), h());
          },

          .parent{this},
      }}

    , m_img
      {{
          .texLoadFunc = []
          {
              return g_progUseDB->retrieve(0X00000371);
          }
      }}

    , m_imgWidget
      {{
          .x = 10,
          .y = 10,

          .w = std::nullopt,
          .h = std::nullopt,

          .parent{this},
      }}

    , m_imgContainer
      {{
          .wrapped{&m_img},
          .parent{&m_imgWidget},
      }}

    , m_imgFrame
      {{
          .w = 200,
          .h = 200,

          .drawFunc = [](const Widget *self, int dstDrawX, int dstDrawY)
          {
              const int w = self->w();
              const int h = self->h();

              const int dx = w / 3;
              const int dy = h / 3;

              g_sdlDevice->drawRectangle(colorf::WHITE + colorf::A_SHF(0X80), dstDrawX, dstDrawY, w, h);

              g_sdlDevice->drawLine(colorf::RED_A255, dstDrawX +     dx, dstDrawY, dstDrawX +     dx, dstDrawY + h - 1);
              g_sdlDevice->drawLine(colorf::RED_A255, dstDrawX + 2 * dx, dstDrawY, dstDrawX + 2 * dx, dstDrawY + h - 1);

              g_sdlDevice->drawLine(colorf::RED_A255, dstDrawX, dstDrawY +     dy, dstDrawX + w - 1, dstDrawY +     dy);
              g_sdlDevice->drawLine(colorf::RED_A255, dstDrawX, dstDrawY + 2 * dy, dstDrawX + w - 1, dstDrawY + 2 * dy);
          },

          .parent{&m_imgWidget},
      }}

    , m_imgResizeHBar
      {{
          .bar
          {
              .x = [this]{ return m_imgFrame.dx(); },
              .y = [this]{ return m_imgFrame.dy() + m_imgFrame.h(); },

              .w = [this]{ return m_imgFrame.w(); },
              .h = 20,

              .v = false,
          },

          .slider
          {
              .w = 20,
              .h = 20,
          },

          .value = 0.5f,

          .barWidget
           {
              .dir = DIR_UPLEFT,
              .widget = new ShapeCropBoard
              {{
                  .w = [this]{ return m_imgFrame.w(); },
                  .h = 20,

                  .drawFunc = [](const Widget *self, int dstDrawX, int dstDrawY)
                  {
                      g_sdlDevice->fillRectangle(colorf::WHITE + colorf::A_SHF(0X80), dstDrawX, dstDrawY, self->w(), self->h());
                  },
              }},
              .autoDelete = true,
           },

          .sliderWidget
           {
              .dir = DIR_UPLEFT,
              .widget = new ShapeCropBoard
              {{
                  .w = 20,
                  .h = 20,

                  .drawFunc = [](const Widget *self, int dstDrawX, int dstDrawY)
                  {
                      g_sdlDevice->fillRectangle(colorf::WHITE + colorf::A_SHF(0X80), dstDrawX, dstDrawY, self->w(), self->h());
                  },
              }},
              .autoDelete = true,
           },
      }}

    , m_imgResizeVBar
      {{
          .bar
          {
              .x = [this]{ return m_imgFrame.dx() + m_imgFrame.w(); },
              .y = [this]{ return m_imgFrame.dy(); },

              .w = 20,
              .h = [this]{ return m_imgFrame.h(); },
          },

          .slider
          {
              .w = 20,
              .h = 20,
          },

          .value = 0.5f,

          .barWidget
           {
              .dir = DIR_UPLEFT,
              .widget = new ShapeCropBoard
              {{
                  .w = 20,
                  .h = [this]{ return m_imgFrame.h(); },

                  .drawFunc = [](const Widget *self, int dstDrawX, int dstDrawY)
                  {
                      g_sdlDevice->fillRectangle(colorf::WHITE + colorf::A_SHF(0X80), dstDrawX, dstDrawY, self->w(), self->h());
                  },
              }},
              .autoDelete = true,
           },

          .sliderWidget
           {
              .dir = DIR_UPLEFT,
              .widget = new ShapeCropBoard
              {{
                  .w = 20,
                  .h = 20,

                  .drawFunc = [](const Widget *self, int dstDrawX, int dstDrawY)
                  {
                      g_sdlDevice->fillRectangle(colorf::WHITE + colorf::A_SHF(0X80), dstDrawX, dstDrawY, self->w(), self->h());
                  },
              }},
              .autoDelete = true,
           },
      }}
{}
