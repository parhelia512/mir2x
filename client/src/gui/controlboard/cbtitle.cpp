#include "widget.hpp"
#include "cbtitle.hpp"
#include "pngtexdb.hpp"

extern PNGTexDB *g_progUseDB;

CBTitle::CBTitle(
        Widget::VarDir argDir,
        Widget::VarInt argX,
        Widget::VarInt argY,

        ProcessRun *argProc,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_processRun(argProc)
    , m_bg
      {{
          .texLoadFunc = [](const Widget *){ return g_progUseDB->retrieve(0X00000022); },
          .parent{this},
      }}

    , m_arcAni
      {
          DIR_UPLEFT,
          54,
          7,

          0X04000000,
          4,
          1,

          true,
          true,

          this,
          false,
      }

    , m_level
      {
          DIR_NONE,
          [this](const Widget *){ return w() / 2; },
          10,

          argProc,
          [this](Widget *, int) // double-click
          {

          },

          this,
          false,
      }
{
    setSize([this]{ return m_bg.w(); },
            [this]{ return m_bg.h(); });
}
