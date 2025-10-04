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
                Widget::VarOff argX,
                Widget::VarOff argY,

                Widget::VarSizeOpt argW,
                Widget::VarSizeOpt argH,

                const Widget *argWidget,

                Widget *argParent     = nullptr,
                bool    argAutoDelete = false)

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

            , m_gfxWidget(argWidget)
        {}

    public:
        const Widget *gfxWidget() const
        {
            return m_gfxWidget;
        }

    public:
        void drawEx(int dstX, int dstY, const Widget::ROIOpt &roi) const override
        {
            const auto roiOpt = cropDrawROI(dstX, dstY, roi);
            if(!roiOpt.has_value()){
                return;
            }

            const auto fnCropDraw = [dstX, dstY, &roiOpt, this](int onCropBrdX, int onCropBrdY, int onCropBrdW, int onCropBrdH, int onScreenX, int onScreenY)
            {
                if(mathf::cropROI(
                            &onCropBrdX, &onCropBrdY,
                            &onCropBrdW, &onCropBrdH,

                            &onScreenX,
                            &onScreenY,

                            m_gfxWidget->w(),
                            m_gfxWidget->h(),

                            0, 0, -1, -1, dstX, dstY, roiOpt->w, roiOpt->h)){
                    m_gfxWidget->drawEx(onScreenX, onScreenY, {onCropBrdX, onCropBrdY, onCropBrdW, onCropBrdH});
                }
            };

            const int extendedDstX = dstX - roiOpt->x;
            const int extendedDstY = dstY - roiOpt->y;

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
