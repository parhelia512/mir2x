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
              return g_progUseDB->retrieve(0X18000001);
          }
      }}

    , m_imgWidget
      {{
          .w = std::nullopt,
          .h = std::nullopt,

          .parent{this},
      }}

    , m_imgContainer
      {{
          .wrapped{&m_img},
          .attrs
          {
              .processEvent = [this](Widget *self, const SDL_Event &event, bool valid, Widget::ROIMap m)
              {
                  if(!m.calibrate(self)){
                      return false;
                  }

                  if((event.type == SDL_MOUSEMOTION) && valid){
                      if((event.motion.state & SDL_BUTTON_LMASK) && (m.in(event.motion.x, event.motion.y) || self->focus())){
                          self->moveBy(event.motion.xrel, event.motion.yrel, m_imgFrame.roi(&m_imgWidget));
                          return true;
                      }
                  }
                  return false;
              },
          },
          .parent{&m_imgWidget},
      }}

    , m_imgFrame
      {{
          .x = 10,
          .y = 10,

          .w = 200,
          .h = 200,

          .drawFunc = [this](const Widget *self, int dstDrawX, int dstDrawY)
          {
              const int w = self->w();
              const int h = self->h();

              const float hr0 = m_cropHSlider_0.getValue();
              const float hr1 = m_cropHSlider_1.getValue();

              const float vr0 = m_cropVSlider_0.getValue();
              const float vr1 = m_cropVSlider_1.getValue();

              g_sdlDevice->drawRectangle(colorf::WHITE + colorf::A_SHF(0X80), dstDrawX, dstDrawY, w, h);

              g_sdlDevice->drawLine(colorf::RED_A255, dstDrawX + hr0 * w, dstDrawY, dstDrawX + hr0 * w, dstDrawY + h - 1);
              g_sdlDevice->drawLine(colorf::RED_A255, dstDrawX + hr1 * w, dstDrawY, dstDrawX + hr1 * w, dstDrawY + h - 1);

              g_sdlDevice->drawLine(colorf::BLUE_A255, dstDrawX, dstDrawY + vr0 * h, dstDrawX + w - 1, dstDrawY + vr0 * h);
              g_sdlDevice->drawLine(colorf::BLUE_A255, dstDrawX, dstDrawY + vr1 * h, dstDrawX + w - 1, dstDrawY + vr1 * h);
          },

          .parent{&m_imgWidget},
      }}

    , m_imgResizeHSlider
      {{
          .bar
          {
              .x = [this]{ return m_imgFrame.dx(); },
              .y = [this]{ return m_imgFrame.dy() + m_imgFrame.h() - 1; },

              .w = [this]{ return m_imgFrame.w(); },
              .h = 1,

              .v = false,
          },

          .slider
          {
              .w = 10,
              .h = 10,
          },

          .value = 0.5f,

          .sliderWidget
           {
              .dir = DIR_UPLEFT,
              .widget = new ShapeCropBoard
              {{
                  .w = 10,
                  .h = 10,

                  .drawFunc = [](const Widget *self, int dstDrawX, int dstDrawY)
                  {
                      g_sdlDevice->fillRectangle(colorf::WHITE + colorf::A_SHF(0X80), dstDrawX, dstDrawY, self->w(), self->h());
                  },
              }},
              .autoDelete = true,
           },

           .parent{&m_imgWidget},
      }}

    , m_imgResizeVSlider
      {{
          .bar
          {
              .x = [this]{ return m_imgFrame.dx() + m_imgFrame.w() - 1; },
              .y = [this]{ return m_imgFrame.dy(); },

              .w = 1,
              .h = [this]{ return m_imgFrame.h(); },
          },

          .slider
          {
              .w = 10,
              .h = 10,
          },

          .value = 0.5f,

          .sliderWidget
           {
              .dir = DIR_UPLEFT,
              .widget = new ShapeCropBoard
              {{
                  .w = 10,
                  .h = 10,

                  .drawFunc = [](const Widget *self, int dstDrawX, int dstDrawY)
                  {
                      g_sdlDevice->fillRectangle(colorf::WHITE + colorf::A_SHF(0X80), dstDrawX, dstDrawY, self->w(), self->h());
                  },
              }},
              .autoDelete = true,
           },

          .parent{&m_imgWidget},
      }}

    , m_cropHSlider_0
      {{
          .bar
          {
              .x = [this]{ return m_imgFrame.dx(); },
              .y = [this]{ return m_imgFrame.dy(); },
              .w = [this]{ return m_imgFrame. w(); },
              .h = 1,
              .v = false,
          },

          .slider
          {
              .w = 10,
              .h = 10,
          },

          .value = 0.333f,

          .sliderWidget
           {
              .dir = DIR_UPLEFT,
              .widget = new ShapeCropBoard
              {{
                  .w = 10,
                  .h = 10,

                  .drawFunc = [](const Widget *self, int dstDrawX, int dstDrawY)
                  {
                      g_sdlDevice->fillRectangle(colorf::WHITE + colorf::A_SHF(0X80), dstDrawX, dstDrawY, self->w(), self->h());
                  },
              }},
              .autoDelete = true,
           },

           .parent{&m_imgWidget},
      }}

    , m_cropHSlider_1
      {{
          .bar
          {
              .x = [this]{ return m_imgFrame.dx(); },
              .y = [this]{ return m_imgFrame.dy(); },
              .w = [this]{ return m_imgFrame. w(); },
              .h = 1,
              .v = false,
          },

          .slider
          {
              .w = 10,
              .h = 10,
          },

          .value = 0.666f,

          .sliderWidget
           {
              .dir = DIR_UPLEFT,
              .widget = new ShapeCropBoard
              {{
                  .w = 10,
                  .h = 10,

                  .drawFunc = [](const Widget *self, int dstDrawX, int dstDrawY)
                  {
                      g_sdlDevice->fillRectangle(colorf::WHITE + colorf::A_SHF(0X80), dstDrawX, dstDrawY, self->w(), self->h());
                  },
              }},
              .autoDelete = true,
           },

           .parent{&m_imgWidget},
      }}

    , m_cropVSlider_0
      {{
          .bar
          {
              .x = [this]{ return m_imgFrame.dx(); },
              .y = [this]{ return m_imgFrame.dy(); },
              .w = 1,
              .h = [this]{ return m_imgFrame. h(); },
          },

          .slider
          {
              .w = 10,
              .h = 10,
          },

          .value = 0.333f,

          .sliderWidget
           {
              .dir = DIR_UPLEFT,
              .widget = new ShapeCropBoard
              {{
                  .w = 10,
                  .h = 10,

                  .drawFunc = [](const Widget *self, int dstDrawX, int dstDrawY)
                  {
                      g_sdlDevice->fillRectangle(colorf::WHITE + colorf::A_SHF(0X80), dstDrawX, dstDrawY, self->w(), self->h());
                  },
              }},
              .autoDelete = true,
           },

           .parent{&m_imgWidget},
      }}

    , m_cropVSlider_1
      {{
          .bar
          {
              .x = [this]{ return m_imgFrame.dx(); },
              .y = [this]{ return m_imgFrame.dy(); },
              .w = 1,
              .h = [this]{ return m_imgFrame. h(); },
          },

          .slider
          {
              .w = 10,
              .h = 10,
          },

          .value = 0.666f,

          .sliderWidget
           {
              .dir = DIR_UPLEFT,
              .widget = new ShapeCropBoard
              {{
                  .w = 10,
                  .h = 10,

                  .drawFunc = [](const Widget *self, int dstDrawX, int dstDrawY)
                  {
                      g_sdlDevice->fillRectangle(colorf::WHITE + colorf::A_SHF(0X80), dstDrawX, dstDrawY, self->w(), self->h());
                  },
              }},
              .autoDelete = true,
           },

           .parent{&m_imgWidget},
      }}
{
    m_img.setSize([this]{ return m_imgFrame.w() * m_imgResizeHSlider.getValue(); },
                  [this]{ return m_imgFrame.h() * m_imgResizeVSlider.getValue(); });

    m_imgContainer.moveTo(m_imgFrame.dx(), m_imgFrame.dy());
}
