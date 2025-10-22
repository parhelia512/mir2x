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
      {
          argDir,
          argX,
          argY,
          argW,
          argH,
          {},

          argParent,
          argAutoDelete
      }

    , m_frame
      {
          DIR_UPLEFT,
          0,
          0,
          {},
          {},
          [](const Widget *){ return g_progUseDB->retrieve(m_frameTexID); },
      }

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
      {
          DIR_UPLEFT,
          argW - 51,
          argH - 53,

          {SYS_U32NIL, 0X0000001C, 0X0000001D},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int)
          {
              this->parent()->setShow(false);
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          this,
      }
{
    fflassert(argW >= m_cornerSize * 2);
    fflassert(argH >= m_cornerSize * 2);
}

void ResizableFrameBoard::draw(Widget::ROIMap) const
{
    const auto roiOpt = cropDrawROI(dstX, dstY, roi);
    if(!roiOpt.has_value()){
        return;
    }

    for(const auto &p:
    {
        static_cast<const Widget *>(&m_frameCropDup),
        static_cast<const Widget *>(&m_close),
    }){
        int drawDstX = dstX;
        int drawDstY = dstY;
        int drawSrcX = roiOpt->x;
        int drawSrcY = roiOpt->y;
        int drawSrcW = roiOpt->w;
        int drawSrcH = roiOpt->h;

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
            p->draw(drawDstX, drawDstY, {drawSrcX, drawSrcY, drawSrcW, drawSrcH});
        }
    }
}
