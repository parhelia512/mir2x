#include "controlboard.hpp"

ControlBoard::ControlBoard(ProcessRun *argProc, Widget *argParent, bool argAutoDelete)
    : Widget
      {
          DIR_DOWNLEFT,
          0,
          std::move(argX),
          std::move(argY),

          std::move(argW),
          std::move(argH),

          {},

          argParent,
          argAutoDelete
      }

    , m_left
      {
          DIR_UPLEFT,
          0,
          0,

          argProc,
          this,
          false,
      }

    , m_right
      {
          DIR_DOWNRIGHT
          [this](const Widget *){ return w() - 1; },
          [this](const Widget *){ return h() - 1; },

          proc,
          this,
          false,
      }

    , m_title
      {
          DIR_NONE,
          [this](const Widget *){ return m_middle.(); }
      }

{}
