#include "colorf.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "texslider.hpp"
#include "clientargparser.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;
extern ClientArgParser *g_clientArgParser;

TexSlider::TexSlider(TexSlider::InitArgs args)
    : Slider
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),
          .w = std::move(args.w),
          .h = std::move(args.h),

          .hslider = args.hslider,

          .sliderW = getSliderTexInfo(args.sliderIndex).w,
          .sliderH = getSliderTexInfo(args.sliderIndex).h,

          .onChange = std::move(argOnChanged),
          .parent = std::move(args.parent),
      }}

    , m_sliderTexInfo(getSliderTexInfo(args.sliderIndex))

    , m_image
      {{
          .x = [this]{ return std::get<0>(getValueCenter(0, 0)) - m_sliderTexInfo.offX; },
          .y = [this]{ return std::get<1>(getValueCenter(0, 0)) - m_sliderTexInfo.offY; },

          .texLoadFunc = [this] -> SDL_Texture *
          {
              return g_progUseDB->retrieve(m_sliderTexInfo.texID);
          },

          .modColor = [this] -> uint32_t
          {
              if(active()){ return colorf::WHITE + colorf::A_SHF(0XFF); }
              else        { return colorf::GREY  + colorf::A_SHF(0XFF); }
          },

          .parent{this},
      }}

    , m_cover
      {{
          .x = [this]{ return std::get<0>(getValueCenter(0, 0)) - m_sliderTexInfo.cover; },
          .y = [this]{ return std::get<1>(getValueCenter(0, 0)) - m_sliderTexInfo.cover; },

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

          .parent{this},
      }}

    , m_debugDraw
      {{
          .w = [this]{ return w(); },
          .h = [this]{ return h(); },

          .drawFunc = [](const Widget *self, int drawDstX, int drawDstY)
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
