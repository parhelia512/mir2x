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

          .childList = std::move(args.childList),
          .parent = std::move(args.parent),
      }}

    , m_bgDrawFunc(std::move(args.bgDrawFunc))
    , m_fgDrawFunc(std::move(args.fgDrawFunc))
{}

void ShapeCropBoard::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }

    if(Widget::hasDrawFunc(m_bgDrawFunc)){
        const SDLDeviceHelper::EnableRenderCropRectangle enableClip(m.x, m.y, m.ro->w, m.ro->h);
        Widget::execDrawFunc(m_bgDrawFunc, this, m.x - m.ro->x, m.y - m.ro->y);
    }

    Widget::drawDefault(m);

    if(Widget::hasDrawFunc(m_fgDrawFunc)){
        const SDLDeviceHelper::EnableRenderCropRectangle enableClip(m.x, m.y, m.ro->w, m.ro->h);
        Widget::execDrawFunc(m_fgDrawFunc, this, m.x - m.ro->x, m.y - m.ro->y);
    }
}
