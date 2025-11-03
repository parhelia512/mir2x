#pragma once
#include "mathf.hpp"
#include "widget.hpp"

class GfxDupBoard: public Widget
{
    private:
        const Widget * const m_gfxWidget;

    public:
        GfxDupBoard(
                Widget::VarDir argDir,
                Widget::VarInt argX,
                Widget::VarInt argY,

                Widget::VarSizeOpt argW,
                Widget::VarSizeOpt argH,

                const Widget *argWidget,

                Widget *argParent     = nullptr,
                bool    argAutoDelete = false)

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

            , m_gfxWidget(argWidget)
        {}

    public:
        const Widget *gfxWidget() const
        {
            return m_gfxWidget;
        }

    public:
        void draw(Widget::ROIMap m) const override
        {
            if(!m.calibrate(this)){
                return;
            }

            const auto fnCropDraw = [dstX = m.x, dstY = m.y, &m, this](int onCropBrdX, int onCropBrdY, int onCropBrdW, int onCropBrdH, int onScreenX, int onScreenY)
            {
                if(mathf::cropROI(
                            &onCropBrdX, &onCropBrdY,
                            &onCropBrdW, &onCropBrdH,

                            &onScreenX,
                            &onScreenY,

                            m_gfxWidget->w(),
                            m_gfxWidget->h(),

                            0, 0, -1, -1, dstX, dstY, m.ro->w, m.ro->h)){
                    m_gfxWidget->draw({.x = onScreenX, .y = onScreenY, .ro{onCropBrdX, onCropBrdY, onCropBrdW, onCropBrdH}});
                }
            };

            const int extendedDstX = m.x - m.ro->x;
            const int extendedDstY = m.y - m.ro->y;

            for(int doneDrawWidth = 0; doneDrawWidth < w();){
                const int drawWidth = std::min<int>(m_gfxWidget->w(), w() - doneDrawWidth);
                for(int doneDrawHeight = 0; doneDrawHeight < h();){
                    const int drawHeight = std::min<int>(m_gfxWidget->h(), h() - doneDrawHeight);
                    fnCropDraw(0, 0, drawWidth, drawHeight, extendedDstX + doneDrawWidth, extendedDstY + doneDrawHeight);
                    doneDrawHeight += drawHeight;
                }
                doneDrawWidth += drawWidth;
            }
        }
};
