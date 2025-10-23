#include "sdldevice.hpp"
#include "shapecropboard.hpp"

ShapeCropBoard::ShapeCropBoard(Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        Widget::VarSizeOpt argW,
        Widget::VarSizeOpt argH,

        VarDrawFunc argDrawFunc,

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

    , m_drawFunc(std::move(argDrawFunc))
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
