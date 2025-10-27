#include "sdldevice.hpp"
#include "shapecropboard.hpp"

ShapeCropBoard::ShapeCropBoard(ShapeCropBoard::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),
          .w = std::move(args.w),
          .h = std::move(args.h),

          .parent = std::move(args.parent),
      }}

    , m_drawFunc(std::move(args.drawFunc))
{}

void ShapeCropBoard::draw(Widget::ROIMap m) const
{
    if(!Widget::hasDrawFunc(m_drawFunc)){
        return;
    }

    if(!m.calibrate(this)){
        return;
    }

    const SDLDeviceHelper::EnableRenderCropRectangle enableClip(m.x, m.y, m.ro->w, m.ro->h);
    Widget::execDrawFunc(m_drawFunc, this, m.x - m.ro->x, m.y - m.ro->y);
}
