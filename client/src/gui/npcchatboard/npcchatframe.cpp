#include "npcchatframe.hpp"

NPCChatFrame::NPCChatFrame(
        Widget::VarDir argDir,
        Widget::VarInt argX,
        Widget::VarInt argY,

        Widget::VarSizeOpt argW,
        Widget::VarSizeOpt argH,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),

          .w = std::move(argW),
          .h = std::move(argH),

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

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
