#include "colorf.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "texslider.hpp"
#include "clientargparser.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;
extern ClientArgParser *g_clientArgParser;

TexSlider::TexSlider(
        Widget::VarDir argDir,
        Widget::VarInt argX,
        Widget::VarInt argY,

        Widget::VarSizeOpt argW,
        Widget::VarSizeOpt argH,

        bool argHSlider,
        int  argSliderIndex,

        std::function<void(float)> argOnChanged,

        Widget *argParent,
        bool    argAutoDelete)

    : Slider
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),
          std::move(argW),
          std::move(argH),

          argHSlider,
          getSliderTexInfo(argSliderIndex).w,
          getSliderTexInfo(argSliderIndex).h,

          std::move(argOnChanged),

          argParent,
          argAutoDelete,
      }

    , m_sliderTexInfo(getSliderTexInfo(argSliderIndex))

    , m_image
      {
          DIR_UPLEFT,
          [this](const Widget *){ return std::get<0>(getValueCenter(0, 0)) - m_sliderTexInfo.offX; },
          [this](const Widget *){ return std::get<1>(getValueCenter(0, 0)) - m_sliderTexInfo.offY; },

          {},
          {},

          [this](const Widget *) -> SDL_Texture *
          {
              return g_progUseDB->retrieve(m_sliderTexInfo.texID);
          },

          false,
          false,
          0,

          [this](const Widget *)
          {
              if(active()){ return colorf::WHITE + colorf::A_SHF(0XFF); }
              else        { return colorf::GREY  + colorf::A_SHF(0XFF); }
          },

          SDL_BLENDMODE_NONE,

          this,
          false,
      }

    , m_cover
      {
          DIR_UPLEFT,
          [this](const Widget *){ return std::get<0>(getValueCenter(0, 0)) - m_sliderTexInfo.cover; },
          [this](const Widget *){ return std::get<1>(getValueCenter(0, 0)) - m_sliderTexInfo.cover; },

          {},
          {},

          [this](const Widget *) -> SDL_Texture *
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

          false,
          false,
          0,

          [this](const Widget *)
          {
              switch(m_sliderState){
                  case BEVENT_ON:
                      {
                          if(active()){ return colorf::BLUE  + colorf::A_SHF(200); }
                          else        { return colorf::WHITE + colorf::A_SHF(255); }
                      }
                  case BEVENT_DOWN:
                      {
                          if(active()){ return colorf::RED   + colorf::A_SHF(200); }
                          else        { return colorf::WHITE + colorf::A_SHF(255); }
                      }
                  default:
                      {
                          return colorf::WHITE + colorf::A_SHF(255);
                      }
              }
          },

          SDL_BLENDMODE_BLEND,

          this,
          false,
      }

    , m_debugDraw
      {
          DIR_UPLEFT,
          0,
          0,

          [this](const Widget *){ return w(); },
          [this](const Widget *){ return h(); },

          [this](const Widget *, int drawDstX, int drawDstY)
          {
              if(g_clientArgParser->debugSlider){
                  g_sdlDevice->drawRectangle(colorf::GREEN + colorf::A_SHF(255), drawDstX, drawDstY, w(), h());

                  const auto [valCenterX, valCenterY] = getValueCenter(drawDstX, drawDstY);
                  g_sdlDevice->drawLine(colorf::YELLOW + colorf::A_SHF(255), valCenterX, valCenterY, valCenterX - m_sliderTexInfo.offX, valCenterY - m_sliderTexInfo.offY);

                  const auto [sliderX, sliderY, sliderW, sliderH] = getSliderRectangle(drawDstX, drawDstY);
                  g_sdlDevice->drawRectangle(colorf::RED + colorf::A_SHF(255), sliderX, sliderY, sliderW, sliderH);
              }
          },

          this,
          false,
      }
{
    fflassert(w() > 0);
    fflassert(h() > 0);
    fflassert(m_sliderTexInfo.w > 0);
    fflassert(m_sliderTexInfo.h > 0);
    fflassert(m_sliderTexInfo.cover > 0);
    fflassert(g_progUseDB->retrieve(m_sliderTexInfo.texID), str_printf("%08X.PNG", m_sliderTexInfo.texID));
}
