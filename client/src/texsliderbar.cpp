#include "pngtexdb.hpp"
#include "imageboard.hpp"
#include "gfxresizeboard.hpp"
#include "texsliderbar.hpp"

extern PNGTexDB *g_progUseDB;

TexSliderBar::TexSliderBar(TexSliderBar::InitArgs args)
    : TexSlider
      {{
          .bar = std::move(args.bar),
          .index = args.index,
          .value = args.value,

          .onChange = std::move(args.onChange),
          .parent = std::move(args.parent),
      }}

    , m_imgSlot
      {{
          .texLoadFunc = []{ return g_progUseDB->retrieve(0X00000460); },
          .rotate = vbar() ? 1 : 0,
      }}

    , m_imgBar
      {{
          .texLoadFunc = []{ return g_progUseDB->retrieve(0X00000470); },
          .rotate = vbar() ? 1 : 0,
      }}

    , m_slot
      {{
          .getter = &m_imgSlot,
          .vr
          {
              /* x */  vbar() ? 0 : 3,
              /* y */ !vbar() ? 0 : 3,
              /* w */  vbar() ? m_imgSlot.w() : m_imgSlot.w() - 6,
              /* h */ !vbar() ? m_imgSlot.h() : m_imgSlot.h() - 6,
          },

          .resize
          {
              [this]{ return getBarROI(0, 0).w; },
              [this]{ return getBarROI(0, 0).h; },
          },
      }}

    , m_bar
      {{
          .w = [this]{ return  vbar() ? m_imgBar.w() : (getBarROI(0, 0).w - 1) * getValue(); },
          .h = [this]{ return !vbar() ? m_imgBar.h() : (getBarROI(0, 0).h - 1) * getValue(); },

          .getter = &m_imgBar,
          .parent{this},
      }}
{
    setBarBgWidget(             0,              0, &m_bar , false);
    setBarBgWidget(vbar() ? 2 : 3, vbar() ? 3 : 2, &m_slot, false);
}
