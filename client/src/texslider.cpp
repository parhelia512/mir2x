#include "colorf.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "texslider.hpp"
#include "clientargparser.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;
extern ClientArgParser *g_clientArgParser;

TexSlider::TexSlider(TexSlider::InitArgs args)
    : Widget
      {{
          .parent = std::move(args.parent),
      }}

    , m_sliderTexInfo(fflcheck(args.index, args.index >= 0 && args.index < static_cast<int>(std::size(m_sliderTexInfoList))))
    , m_image
      {{
          .texLoadFunc = [this] -> SDL_Texture *
          {
              return g_progUseDB->retrieve(m_sliderTexInfo->texID);
          },

          .modColor = [this] -> uint32_t
          {
              if(active()){ return colorf::WHITE + colorf::A_SHF(0XFF); }
              else        { return colorf::GREY  + colorf::A_SHF(0XFF); }
          },
      }}

    , m_cover
      {{
          .texLoadFunc = [this] -> SDL_Texture *
          {
              switch(m_sliderState){
                  case BEVENT_ON:
                  case BEVENT_DOWN:
                      {
                          return g_sdlDevice->getCover(m_sliderTexInfo.cover, 360);
                      }
                  default:
                      {
                          return nullptr;
                      }
              }
          },

          .modColor = [this] -> uint32_t
          {
              switch(m_sliderState){
                  case BEVENT_ON:
                      {
                          if(active()){ return colorf::BLUE  + colorf::A_SHF(200); }
                          else        { return colorf::WHITE_A255; }
                      }
                  case BEVENT_DOWN:
                      {
                          if(active()){ return colorf::RED   + colorf::A_SHF(200); }
                          else        { return colorf::WHITE_A255; }
                      }
                  default:
                      {
                          return colorf::WHITE_A255;
                      }
              }
          },
      }}

    , m_sliderWidget
      {{
          .childList
          {
              {std::addressof(m_image)},
              {std::addressof(m_cover)},
          },
      }}

    , m_base
      {{
          .bar = std::move(args.bar)
          .slider
          {
              .cx = [this]{ return m_sliderTexInfo->offX; },
              .cy = [this]{ return m_sliderTexInfo->offY; },

              .w = [this]{ return SDLDeviceHelper::getTextureWidth (g_progUseDB->retrieve(m_sliderTexInfo->texID)); },
              .h = [this]{ return SDLDeviceHelper::getTextureHeight(g_progUseDB->retrieve(m_sliderTexInfo->texID)); },
          },

          .value = args.value,
          .sliderWidget
          {
              .dir = DIR_UPLEFT,
              .widget = std::addressof(m_sliderWidget),
          },

          .onChange = std::move(args.onChange),
          .parent{this},
      }}

    , m_debugDraw
      {{
          .w = [this]{ return w(); },
          .h = [this]{ return h(); },

          .drawFunc = [this](const Widget *self, int drawDstX, int drawDstY)
          {
              if(g_clientArgParser->debugSlider){
                  g_sdlDevice->drawRectangle(colorf::GREEN_A255, drawDstX, drawDstY, self->w(), self->h());

                  const auto [valCenterX, valCenterY] = getValueCenter(drawDstX, drawDstY);
                  g_sdlDevice->drawLine(colorf::YELLOW_A255, valCenterX, valCenterY, valCenterX - m_sliderTexInfo.offX, valCenterY - m_sliderTexInfo.offY);

                  const auto [sliderX, sliderY, sliderW, sliderH] = getSliderRectangle(drawDstX, drawDstY);
                  g_sdlDevice->drawRectangle(colorf::RED_A255, sliderX, sliderY, sliderW, sliderH);
              }
          },

          .parent{this},
      }}
{
    fflassert(w() > 0);
    fflassert(h() > 0);
    fflassert(m_sliderTexInfo.w > 0);
    fflassert(m_sliderTexInfo.h > 0);
    fflassert(m_sliderTexInfo.cover > 0);
    fflassert(g_progUseDB->retrieve(m_sliderTexInfo.texID), str_printf("%08X.PNG", m_sliderTexInfo.texID));
}
