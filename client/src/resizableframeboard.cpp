#include "colorf.hpp"
#include "totype.hpp"
#include "imageboard.hpp"
#include "gfxcropboard.hpp"
#include "gfxdupboard.hpp"
#include "resizableframeboard.hpp"

extern PNGTexDB *g_progUseDB;

ResizableFrameBoard::ResizableFrameBoard(
        dir8_t argDir,
        int argX,
        int argY,
        int argW,
        int argH,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = argDir,

          .x = argX,
          .y = argY,
          .w = argW,
          .h = argH,

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_frame
      {{
          .texLoadFunc = [](const Widget *){ return g_progUseDB->retrieve(m_frameTexID); },
      }}

    , m_frameCropDup
      {
          DIR_UPLEFT,
          0,
          0,
          argW,
          argH,

          &m_frame,

          m_cornerSize,
          m_cornerSize,
          m_frame.w() - 2 * m_cornerSize,
          m_frame.h() - 2 * m_cornerSize,

          this,
          false,
      }

    , m_close
      {{
          .x = argW - 51,
          .y = argH - 53,

          .texIDList
          {
              .on   = 0X0000001C,
              .down = 0X0000001D,
          },

          .onTrigger = [this](Widget *, int)
          {
              this->parent()->setShow(false);
          },

          .parent{this},
      }}
{
    fflassert(argW >= m_cornerSize * 2);
    fflassert(argH >= m_cornerSize * 2);
}

void ResizableFrameBoard::draw(Widget::ROIMap m) const
{
    if(!m.crop(roi())){
        return;
    }

    for(const auto &p:
    {
        static_cast<const Widget *>(&m_frameCropDup),
        static_cast<const Widget *>(&m_close),
    }){
        int drawDstX = m.x;
        int drawDstY = m.y;
        int drawSrcX = m.ro->x;
        int drawSrcY = m.ro->y;
        int drawSrcW = m.ro->w;
        int drawSrcH = m.ro->h;

        if(mathf::cropChildROI(
                    &drawSrcX, &drawSrcY,
                    &drawSrcW, &drawSrcH,
                    &drawDstX, &drawDstY,

                    w(),
                    h(),

                    p->dx(),
                    p->dy(),
                    p-> w(),
                    p-> h())){
            p->draw({.x=drawDstX, .y=drawDstY, .ro{drawSrcX, drawSrcY, drawSrcW, drawSrcH}});
        }
    }
}
