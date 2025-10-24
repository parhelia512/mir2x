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
    if((m_drawFunc.index() == 0) && !std::get<0>(m_drawFunc)){ return; }
    if((m_drawFunc.index() == 1) && !std::get<1>(m_drawFunc)){ return; }

    if(!m.crop(roi())){
        return;
    }

    const SDLDeviceHelper::EnableRenderCropRectangle enableClip(m.x, m.y, m.ro->w, m.ro->h);
    switch(m_drawFunc.index()){
        case 0:
            {
                std::get<0>(m_drawFunc)(m.x - m.ro->x, m.y - m.ro->y);
                return;
            }
        case 1:
            {
                std::get<1>(m_drawFunc)(this, m.x - m.ro->x, m.y - m.ro->y);
                return;
            }
        default:
            {
                return;
            }
    }
}
