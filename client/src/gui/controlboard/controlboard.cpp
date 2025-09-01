#include "controlboard.hpp"

SDLDevice *g_sdlDevice;

ControlBoard::ControlBoard(ProcessRun *argProc, Widget *argParent, bool argAutoDelete)
    : Widget
      {
          DIR_DOWNLEFT,

          0,
          [this](const Widget *){ return g_sdlDevice->getRendererHeight() - 1; },
          [this](const Widget *){ return g_sdlDevice->getRendererWidth ()    ; },
          [this](const Widget *)
          {
              switch(m_mode){
                  case CBM_HIDE:
                      {
                          return CBTitle::UP_HEIGHT;
                      }
                  case CBM_DEF:
                      {
                          return m_middle.h() + CBTitle::UP_HEIGHT;
                      }
                  default:
                      {
                          return m_middleExpand.h() + CBTitle::UP_HEIGHT;
                      }
              }
          },

          {},

          argParent,
          argAutoDelete
      }

    , m_left
      {
          DIR_DOWNLEFT,
          0,
          [this](const Widget *){ return h() - 1; },

          argProc,
          this,
          false,
      }

    , m_right
      {
          DIR_DOWNRIGHT
          [this](const Widget *){ return w() - 1; },
          [this](const Widget *){ return h() - 1; },

          argProc,
          this,
          false,
      }

    , m_middle
      {
          DIR_DOWNLEFT,
          [this](const Widget *)
          {
              return m_left.w();
          },

          [this](const Widget *)
          {
              return h() - 1;
          },

          [this](const Widget *)
          {
              return w() - m_left.w() - m_right.w();
          },

          argProc,
          this,
          false,
      }

    , m_middleExpand
      {
          DIR_DOWNLEFT,
          [this](const Widget *)
          {
              return m_left.w();
          },

          [this](const Widget *)
          {
              return h() - 1;
          },

          [this](const Widget *)
          {
              return w() - m_left.w() - m_right.w();
          },

          {},

          argProc,
          this,
          false,
      }

    , m_title
      {
          DIR_UP,
          [this](const Widget *)
          {
              return (w() - m_left.w() - m_right.w()) / 2;
          },

          [this](const Widget *)
          {
              return CBTitle::UP_HEIGHT;
          },

          argProc,
          this,
          false,
      }
{}
