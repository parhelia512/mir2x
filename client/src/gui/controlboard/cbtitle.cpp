#include "cbtitle.hpp"

CBTitle::CBTitle(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        ProcessRun *argProc,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),

          {},
          {},
          {},

          argParent,
          argAutoDelete,
      }

    , m_bg
      {
          DIR_UPLEFT,
          0,
          0,

          {},
          {},

          [](const Widget *))
          {
              return g_progUseDB->retrieve(0X00000022);
          },

          false,
          false,
          0,

          colorf::WHITE + colorf::A_SHF(255),

          this,
          false,
      }

    , m_arcAni
      {
          DIR_UP,
          20,
          30,

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
          [this](const Widget *){ return w() / 2; }
          10,

          argProc,
          [this]() // double-click
          {

          },

          this,
          false,
      }

