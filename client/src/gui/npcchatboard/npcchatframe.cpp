#include "npcchatframe.hpp"

NPCChatFrame::NPCChatFrame(
        Widget::VarDir argDir,
        Widget::VarInt argX,
        Widget::VarInt argY,

        Widget::VarOptSize argW,
        Widget::VarOptSize argH,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),

          std::move(argW),
          std::move(argH),

          {},

          argParent,
          argAutoDelete,
      }

    , m_frame
      {
          DIR_UPLEFT,
          0,
          0,
      }

    , m_board
      {
          DIR_UPLEFT,
          0,
          0,

          [this](const Widget *){ return w(); },
          [this](const Widget *){ return h(); },

          &m_frame,

          40,
          50,

          300,
          110,

          this,
          false,
      }
{}
