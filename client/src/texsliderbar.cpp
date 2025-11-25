#include "pngtexdb.hpp"
#include "texsliderbar.hpp"

extern PNGTexDB *g_progUseDB;
TexSliderBar::TexSliderBar(
        dir8_t argDir,

        int argX,
        int argY,
        int argSize,

        bool argHSlider,
        int  argSliderIndex,

        std::function<void(float)> argOnChanged,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = argDir,
          .x = argX,
          .y = argY,

          .w =  argHSlider ? [argSize]{ fflassert(argSize >= 6, argSize); return argSize; }() : SDLDeviceHelper::getTextureHeight(g_progUseDB->retrieve(0X00000460)),
          .h = !argHSlider ? [argSize]{ fflassert(argSize >= 6, argSize); return argSize; }() : SDLDeviceHelper::getTextureHeight(g_progUseDB->retrieve(0X00000460)),

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_slotImage
      {{
          .texLoadFunc = []{ return g_progUseDB->retrieve(0X00000460); },
          .rotate = argHSlider ? 0 : 1,
      }}

    , m_barImage
      {{
          .texLoadFunc = []{ return g_progUseDB->retrieve(0X00000470); },
          .rotate = argHSlider ? 0 : 1,
      }}

    , m_slotCropLeft
      {{
          .getter = &m_slotImage,
          .vr
          {
          /* brdCropW */  argHSlider ? 3 : m_slotImage.w(),
          /* brdCropH */ !argHSlider ? 3 : m_slotImage.h(),
          },
          .parent{this},
      }}

    , m_slotCropMiddle
      {{
          .getter = &m_slotImage,
          .vr
          {
              /* brdCropX */  argHSlider ? 3 : 0,
              /* brdCropY */ !argHSlider ? 3 : 0,

              /* brdCropW */  argHSlider ? m_slotImage.w() - 6 : m_slotImage.w(),
              /* brdCropH */ !argHSlider ? m_slotImage.h() - 6 : m_slotImage.h(),
          },
      }}

    , m_slotCropRight
      {{
          .dir = DIR_DOWNRIGHT,
          .x = w() - 1,
          .y = h() - 1,

          .getter = &m_slotImage,
          .vr
          {
              /* brdCropX */  argHSlider ? m_slotImage.w() - 3 : 0,
              /* brdCropY */ !argHSlider ? m_slotImage.h() - 3 : 0,

              /* brdCropW */  argHSlider ? 3 : m_slotImage.w(),
              /* brdCropW */ !argHSlider ? 3 : m_slotImage.h(),
          },
          .parent{this},
      }}

    , m_slotMidCropDup
      {{
          /* x */ .x =  argHSlider ? 3 : 0,
          /* y */ .y = !argHSlider ? 3 : 0,

          /* w */ .w =  argHSlider ? w() - 6 : w(),
          /* h */ .h = !argHSlider ? h() - 6 : h(),

          .getter = &m_slotCropMiddle,
          .parent{this},
      }}

    , m_barCropDup
      {{
          /* x */ .x =  argHSlider ? 3 : 2,
          /* y */ .y = !argHSlider ? 3 : 2,

          /* w */ .w =  argHSlider ? 0 : m_barImage.w(),
          /* h */ .h = !argHSlider ? 0 : m_barImage.h(),

          .getter = &m_barImage,
          .parent{this},
      }}

    , m_slider
      {{
          .bar
          {
              .dir = DIR_NONE,

              .x = w() / 2,
              .y = h() / 2,
              .w = w() - 6,
              .h = h() - 6,
              .v = !argHSlider,
          },
          .index = argSliderIndex,

          .onChange = [argHSlider, argOnChanged = std::move(argOnChanged), this](float val)
          {
              argOnChanged(val);
              if(argHSlider){
                  m_barCropDup.setW(to_dround(val * (w() - 6)));
              }
              else{
                  m_barCropDup.setH(to_dround(val * (h() - 6)));
              }
          },

          .parent{this},
      }}
{}
