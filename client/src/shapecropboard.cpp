#include "sdldevice.hpp"
#include "shapecropboard.hpp"

ShapeCropBoard::ShapeCropBoard(Widget::VarDir argDir,
        Widget::VarInt argX,
        Widget::VarInt argY,

        Widget::VarOptSize argW,
        Widget::VarOptSize argH,

        std::function<void(const Widget *, int, int)> argDrawFunc,

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

    , m_drawFunc(std::move(argDrawFunc))
{}

void ShapeCropBoard::drawEx(int dstX, int dstY, const Widget::ROIOpt &roi) const
{
    if(!m_drawFunc){
        return;
    }

    const auto roiOpt = cropDrawROI(dstX, dstY, roi);
    if(!roiOpt.has_value()){
        return;
    }

    const SDLDeviceHelper::EnableRenderCropRectangle enableClip(dstX, dstY, roiOpt->w, roiOpt->h);
    m_drawFunc(this, dstX - roiOpt->x, dstY - roiOpt->y);
}
