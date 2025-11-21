#include "colorf.hpp"
#include "totype.hpp"
#include "pngtexdb.hpp"
#include "textboard.hpp"
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

    , m_srcWidget
      {{
          .x = 0,
          .y = 0,
          .w = 200,
          .h = h(),

          .parent{this},
      }}

    , m_imgCanvas
      {{
          .x = 20,
          .y = 20,
          .w = [this]{ return m_srcWidget.w() - 40; },
          .h = 120,

          .parent{&m_srcWidget},
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
                          self->moveBy(event.motion.xrel, event.motion.yrel, m_imgCanvas.roi());
                          return true;
                      }
                  }
                  return false;
              },
          },
          .parent{&m_imgCanvas},
      }}

    , m_imgFrame
      {{
          .w = [this]{ return m_imgCanvas.w(); },
          .h = [this]{ return m_imgCanvas.h(); },

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

          .parent{&m_imgCanvas},
      }}

    , m_imgResizeHSlider
      {{
          .bar
          {
              .x = [this]{ return m_imgCanvas.dx(); },
              .y = [this]{ return m_imgCanvas.dy(); },
              .w = [this]{ return m_imgCanvas. w(); },
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

          .onChange = [this](float)
          {
              m_imgContainer.moveBy(0, 0, m_imgCanvas.roi());
          },

          .parent{&m_srcWidget},
      }}

    , m_imgResizeVSlider
      {{
          .bar
          {
              .x = [this]{ return m_imgCanvas.dx(); },
              .y = [this]{ return m_imgCanvas.dy(); },
              .w = 1,
              .h = [this]{ return m_imgCanvas.h(); },
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

          .onChange = [this](float)
          {
              m_imgContainer.moveBy(0, 0, m_imgCanvas.roi());
          },

          .parent{&m_srcWidget},
      }}

    , m_cropHSlider_0
      {{
          .bar
          {
              .x = [this]{ return m_imgCanvas.dx(); },
              .y = [this]{ return m_imgCanvas.dy() + m_imgCanvas.h() - 1; },
              .w = [this]{ return m_imgCanvas.w(); },
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

           .parent{&m_srcWidget},
      }}

    , m_cropHSlider_1
      {{
          .bar
          {
              .x = [this]{ return m_imgCanvas.dx(); },
              .y = [this]{ return m_imgCanvas.dy() + m_imgCanvas.h() - 1; },
              .w = [this]{ return m_imgCanvas.w(); },
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

           .parent{&m_srcWidget},
      }}

    , m_cropVSlider_0
      {{
          .bar
          {
              .x = [this]{ return m_imgCanvas.dx() + m_imgCanvas.w() - 1; },
              .y = [this]{ return m_imgCanvas.dy(); },
              .w = 1,
              .h = [this]{ return m_imgCanvas. h(); },
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

          .parent{&m_srcWidget},
      }}

    , m_cropVSlider_1
      {{
          .bar
          {
              .x = [this]{ return m_imgCanvas.dx() + m_imgCanvas.w() - 1; },
              .y = [this]{ return m_imgCanvas.dy(); },
              .w = 1,
              .h = [this]{ return m_imgCanvas. h(); },
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

          .parent{&m_srcWidget},
      }}

    , m_imgSize
      {{
          .x = [this]{ return m_imgCanvas.dx(); },
          .y = [this]{ return m_imgCanvas.dy() + m_imgCanvas.h() + 20; },

          .textFunc = [this]
          {
              return str_printf("IMG (%d, %d)", m_img.w(), m_img.h());
          },

          .font{.size = 12},
          .parent{&m_srcWidget},
      }}

    , m_roiInfo
      {{
          .x = [this]{ return m_imgSize.dx(); },
          .y = [this]{ return m_imgSize.dy() + m_imgSize.h() + 20; },

          .textFunc = [this]
          {
              const auto r = getROI();
              return str_printf("ROI (%d, %d, %d, %d)", r.x, r.y, r.w, r.h);
          },

          .font{.size = 12},
          .parent{&m_srcWidget},
      }}

    , m_dstWidget
      {{
          .x = [this]{ return       m_srcWidget.dx() + m_srcWidget.w(); },
          .w = [this]{ return w() - m_srcWidget.dx() - m_srcWidget.w(); },
          .h = h(),

          .parent{this},
      }}

    , m_dstCanvas
      {{
          .x = 20,
          .y = 20,
          .w = [this]{ return m_dstWidget.w() - 40; },
          .h = [this]{ return m_dstWidget.h() - 40; },

          .parent{&m_dstWidget},
      }}

    , m_resizeBoard
      {{
          .getter = &m_img,
          .bgDrawFunc = [](const Widget *self, int dstDrawX, int dstDrawY)
          {
              g_sdlDevice->fillRectangle(colorf::BLACK + colorf::A_SHF(0XF0), dstDrawX, dstDrawY, self->w(), self->h());
              g_sdlDevice->drawRectangle(colorf::WHITE + colorf::A_SHF(0X80), dstDrawX, dstDrawY, self->w(), self->h());
          },
          .parent{&m_dstCanvas},
      }}

    , m_marginHSlider_0
      {{
          .bar
          {
              .x = [this]{ return m_dstCanvas.dx()                      ; },
              .y = [this]{ return m_dstCanvas.dy() + m_dstCanvas.h() - 1; },
              .w = [this]{ return                    m_dstCanvas.w()    ; },
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

           .parent{&m_dstWidget},
      }}

    , m_marginHSlider_1
      {{
          .bar
          {
              .x = [this]{ return m_dstCanvas.dx()                      ; },
              .y = [this]{ return m_dstCanvas.dy() + m_dstCanvas.h() - 1; },
              .w = [this]{ return                    m_dstCanvas.w()    ; },
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

           .parent{&m_dstWidget},
      }}

    , m_marginVSlider_0
      {{
          .bar
          {
              .x = [this]{ return m_dstCanvas.dx() + m_dstCanvas.w() - 1; },
              .y = [this]{ return m_dstCanvas.dy()                      ; },
              .w = 1,
              .h = [this]{ return                    m_dstCanvas.h()    ; },
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

          .parent{&m_dstWidget},
      }}

    , m_marginVSlider_1
      {{
          .bar
          {
              .x = [this]{ return m_dstCanvas.dx() + m_dstCanvas.w() - 1; },
              .y = [this]{ return m_dstCanvas.dy()                      ; },
              .w = 1,
              .h = [this]{ return                    m_dstCanvas. h()   ; },
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

          .parent{&m_dstWidget},
      }}
{
    m_img.setSize([this]{ return m_imgCanvas.w() * m_imgResizeHSlider.getValue(); },
                  [this]{ return m_imgCanvas.h() * m_imgResizeVSlider.getValue(); });

    m_imgContainer.moveTo(m_imgFrame.dx(), m_imgFrame.dy());
}

Widget::ROI GfxDebugBoard::getROI() const
{
    const auto canvas_w = m_imgCanvas.w();
    const auto canvas_h = m_imgCanvas.h();

    const float hr0 = m_cropHSlider_0.getValue();
    const float hr1 = m_cropHSlider_1.getValue();

    const float vr0 = m_cropVSlider_0.getValue();
    const float vr1 = m_cropVSlider_1.getValue();
    //
    const auto x0 = static_cast<int>(canvas_w * hr0);
    const auto x1 = static_cast<int>(canvas_w * hr1);

    const auto y0 = static_cast<int>(canvas_h * vr0);
    const auto y1 = static_cast<int>(canvas_h * vr1);

    return Widget::ROI
    {
        .x = std::min<int>(x0, x1) - m_imgContainer.dx(),
        .y = std::min<int>(y0, y1) - m_imgContainer.dy(),

        .w = std::abs(x0 - x1),
        .h = std::abs(y0 - y1),
    };
}
