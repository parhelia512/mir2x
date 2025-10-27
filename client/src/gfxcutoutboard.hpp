#pragma once
#include <initializer_list>
#include "fflerror.hpp"
#include "mathf.hpp"
#include "widget.hpp"
#include "gfxcropboard.hpp"

class GfxCutoutBoard: public Widget
{
    private:
        const Widget * const m_gfxWidget;

    private:
        const int m_cropX;
        const int m_cropY;
        const int m_cropW;
        const int m_cropH;

    public:
        GfxCutoutBoard(
                Widget::VarDir argDir,
                Widget::VarOff argX,
                Widget::VarOff argY,

                const Widget *argWidget,

                int argCropX,
                int argCropY,
                int argCropW,
                int argCropH,

                Widget *argParent     = nullptr,
                bool    argAutoDelete = false)

            : Widget
              {{
                  .dir = std::move(argDir),

                  .x = std::move(argX),
                  .y = std::move(argY),
                  .w = [argWidget]{ return argWidget->w(); },
                  .h = [argWidget]{ return argWidget->h(); },

                  .parent
                  {
                      .widget = argParent,
                      .autoDelete = argAutoDelete,
                  }
              }}

            , m_gfxWidget(fflcheck(argWidget))

            , m_cropX(argCropX)
            , m_cropY(argCropY)
            , m_cropW(fflcheck(argCropW, argCropW >= 0))
            , m_cropH(fflcheck(argCropH, argCropH >= 0))
        {}

    public:
        void draw(Widget::ROIMap m) const override
        {
            if(!m.calibrate(this)){
                return;
            }

            int cropSrcX = m_cropX;
            int cropSrcY = m_cropY;
            int cropSrcW = m_cropW;
            int cropSrcH = m_cropH;

            if(!mathf::rectangleOverlapRegion<int>(0, 0, m_gfxWidget->w(), m_gfxWidget->h(), cropSrcX, cropSrcY, cropSrcW, cropSrcH)){
                return;
            }

            //       w0  w1  w2
            //     +---+---+---+
            //  h0 |           |
            //     +---+---+---+
            //  h1 |   |   |   |
            //     +---+---+---+
            //  h2 |           |
            //     +---+---+---+

            const int x0 = 0;
            const int y0 = 0;
            const int w0 = cropSrcX;
            const int h0 = cropSrcY;

            const int x1 = x0 + w0;
            const int y1 = y0 + h0;
            const int w1 = cropSrcW;
            const int h1 = cropSrcH;

            const int x2 = x1 + w1;
            const int y2 = y1 + h1;
            const int w2 = m_gfxWidget->w() - w0 - w1;
            const int h2 = m_gfxWidget->h() - h0 - h1;

            const GfxCropBoard top    {DIR_UPLEFT, x0, y0, m_gfxWidget, x0, y0, w0 + w1 + w2, h0};
            const GfxCropBoard left   {DIR_UPLEFT, x0, y1, m_gfxWidget, x0, y1, w0          , h1};
            const GfxCropBoard right  {DIR_UPLEFT, x2, y1, m_gfxWidget, x2, y1,           w2, h1};
            const GfxCropBoard bottom {DIR_UPLEFT, x0, y2, m_gfxWidget, x0, y2, w0 + w1 + w2, h2};

            for(auto &p:
            {
                static_cast<const Widget *>(&top   ),
                static_cast<const Widget *>(&left  ),
                static_cast<const Widget *>(&right ),
                static_cast<const Widget *>(&bottom),
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
};
