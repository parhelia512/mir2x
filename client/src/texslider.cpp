#include "colorf.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "texslider.hpp"
#include "clientargparser.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;
extern ClientArgParser *g_clientArgParser;

TexSlider::TexSlider(TexSlider::InitArgs args)
    : SliderBase
      {{
          .bar = std::move(args.bar),
          .slider
          {
              .cx = [index = args.index, this]{ return TexSlider::getSliderTexInfo(index)->offX; },
              .cy = [index = args.index, this]{ return TexSlider::getSliderTexInfo(index)->offY; },

              .w = [index = args.index, this]{ return SDLDeviceHelper::getTextureWidth (g_progUseDB->retrieve(TexSlider::getSliderTexInfo(index)->texID)); },
              .h = [index = args.index, this]{ return SDLDeviceHelper::getTextureHeight(g_progUseDB->retrieve(TexSlider::getSliderTexInfo(index)->texID)); },
          },

          .value = args.value,
          .sliderWidget
          {
              .dir = DIR_UPLEFT,
              .widget = new Widget
              {{
                  .w = {},
                  .h = {},

                  .childList
                  {
                      {new ImageBoard
                      {{
                          .texLoadFunc = [index = args.index, this] -> SDL_Texture *
                          {
                              return g_progUseDB->retrieve(TexSlider::getSliderTexInfo(index)->texID);
                          },

                          .modColor = [this] -> uint32_t
                          {
                              if(active()){ return colorf::WHITE + colorf::A_SHF(0XFF); }
                              else        { return colorf::GREY  + colorf::A_SHF(0XFF); }
                          },

                      }}, DIR_UPLEFT, 0, 0, true},

                      {new ImageBoard
                      {{
                          .dir = DIR_NONE,

                          .x = [index = args.index, this]{ return TexSlider::getSliderTexInfo(index)->offX; },
                          .y = [index = args.index, this]{ return TexSlider::getSliderTexInfo(index)->offY; },

                          .texLoadFunc = [index = args.index, this] -> SDL_Texture *
                          {
                              switch(sliderState()){
                                  case BEVENT_ON:
                                  case BEVENT_DOWN:
                                      {
                                          return g_sdlDevice->getCover(TexSlider::getSliderTexInfo(index)->cover, 360);
                                      }
                                  default:
                                      {
                                          return nullptr;
                                      }
                              }
                          },

                          .modColor = [this] -> uint32_t
                          {
                              switch(sliderState()){
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
                      }}, DIR_UPLEFT, 0, 0, true},
                  }
              }},
          },

          .onChange = std::move(args.onChange),
          .parent = std::move(args.parent),
      }}
{
    fflassert(w() > 0);
    fflassert(h() > 0);

    const auto sliderInfo = TexSlider::getSliderTexInfo(args.index);

    fflassert(sliderInfo->cover > 0);
    fflassert(g_progUseDB->retrieve(sliderInfo->texID), str_printf("%08X.PNG", sliderInfo->texID));
}
