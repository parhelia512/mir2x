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
          .roi
          {
          /* brdCropW */ .w =  argHSlider ? 3 : m_slotImage.w(),
          /* brdCropH */ .h = !argHSlider ? 3 : m_slotImage.h(),
          },
          .parent{this},
      }}

    , m_slotCropMiddle
      {{
          .getter = &m_slotImage,
          .roi
          {
              /* brdCropX */ .x =  argHSlider ? 3 : 0,
              /* brdCropY */ .y = !argHSlider ? 3 : 0,

              /* brdCropW */ .w =  argHSlider ? m_slotImage.w() - 6 : m_slotImage.w(),
              /* brdCropH */ .h = !argHSlider ? m_slotImage.h() - 6 : m_slotImage.h(),
          },
      }}

    , m_slotCropRight
      {{
          .dir = DIR_DOWNRIGHT,
          .x = w() - 1,
          .y = h() - 1,

          .getter = &m_slotImage,
          .roi
          {
              /* brdCropX */ .x =  argHSlider ? m_slotImage.w() - 3 : 0,
              /* brdCropY */ .y = !argHSlider ? m_slotImage.h() - 3 : 0,

              /* brdCropW */ .w =  argHSlider ? 3 : m_slotImage.w(),
              /* brdCropW */ .h = !argHSlider ? 3 : m_slotImage.h(),
          },
          .parent{this},
      }}

    , m_slotMidCropDup
      {
          DIR_UPLEFT,

          /* x */  argHSlider ? 3 : 0,
          /* y */ !argHSlider ? 3 : 0,

          /* w */  argHSlider ? w() - 6 : w(),
          /* h */ !argHSlider ? h() - 6 : h(),

          &m_slotCropMiddle,

          this,
          false,
      }

    , m_barCropDup
      {
          DIR_UPLEFT,

          /* x */  argHSlider ? 3 : 2,
          /* y */ !argHSlider ? 3 : 2,

          /* w */  argHSlider ? 0 : m_barImage.w(),
          /* h */ !argHSlider ? 0 : m_barImage.h(),

          &m_barImage,

          this,
          false,
      }

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
