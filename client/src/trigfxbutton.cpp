#include "colorf.hpp"
#include "trigfxbutton.hpp"

TrigfxButton::TrigfxButton(Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        std::array<const Widget *, 3> argGfxList,
        std::array<std::optional<uint32_t>, 3> argSeffIDList,

        std::function<void(Widget *      )> argOnOverIn,
        std::function<void(Widget *      )> argOnOverOut,
        std::function<void(Widget *, bool, int)> argOnClick,
        std::function<void(Widget *,      int)> argOnTrigger,

        int argOffXOnOver,
        int argOffYOnOver,
        int argOffXOnClick,
        int argOffYOnClick,

        bool argOnClickDone,
        bool argRadioMode,

        Widget *argParent,
        bool    argAutoDelete)

    : ButtonBase
      {
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),

          .onOverIn  = std::move(argOnOverIn),
          .onOverOut = std::move(argOnOverOut),

          .onClick = std::move(argOnClick),
          .onTrigger = std::move(argOnTrigger),

          .seff
          {
              .onOverIn  = argSeffIDList[0],
              .onOverOut = argSeffIDList[1],
              .onClick   = argSeffIDList[2],
          },

          .offXOnOver = argOffXOnOver,
          .offYOnOver = argOffYOnOver,

          .offXOnClick = argOffXOnClick,
          .offXOnClick = argOffYOnClick,

          .onClickDone = argOnClickDone,
          .radioMode = argRadioMode,

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }

    , m_gfxList
      {
          argGfxList[0],
          argGfxList[1],
          argGfxList[2],
      }
{
    initButtonSize();
}

void TrigfxButton::draw(Widget::ROIMap m) const
{
    if(!m.crop(roi())){
        return;
    }

    if(auto gfxPtr = m_gfxList[getState()]){
        m.x += m_offset[getState()][0];
        m.y += m_offset[getState()][1];
        gfxPtr->draw(m);
    }
}

void TrigfxButton::initButtonSize()
{
    int maxW = 0;
    int maxH = 0;

    for(const auto &p: m_gfxList){
        maxW = std::max<int>(p->w(), maxW);
        maxH = std::max<int>(p->h(), maxH);
    }

    setW(maxW);
    setH(maxH);
}
