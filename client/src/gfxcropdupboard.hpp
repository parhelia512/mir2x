#pragma once
#include <initializer_list>
#include "mathf.hpp"
#include "widget.hpp"
#include "gfxdupboard.hpp"
#include "gfxcropboard.hpp"

class GfxCropDupBoard: public Widget
{
    private:
        Widget * m_gfxWidget;

    private:
        const Widget::VarInt m_cropX;
        const Widget::VarInt m_cropY;
        const Widget::VarInt m_cropW;
        const Widget::VarInt m_cropH;

    public:
        GfxCropDupBoard(
                Widget::VarDir argDir,
                Widget::VarInt argX,
                Widget::VarInt argY,

                Widget::VarSizeOpt argW,
                Widget::VarSizeOpt argH,

                Widget *argWidget,

                Widget::VarInt argCropX,
                Widget::VarInt argCropY,
                Widget::VarInt argCropW, // don't use VarSizeOpt because empty VarSizeOpt is not well-defined
                Widget::VarInt argCropH, // ...

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

            , m_gfxWidget([argWidget]{ fflassert(argWidget); return argWidget; }())

            , m_cropX(std::move(argCropX))
            , m_cropY(std::move(argCropY))
            , m_cropW(std::move(argCropW))
            , m_cropH(std::move(argCropH))
        {}

    public:
        void draw(Widget::ROIMap m) const override
        {
            if(!m.calibrate(this)){
                return;
            }

            int cropSrcX = Widget::evalInt(m_cropX, this);
            int cropSrcY = Widget::evalInt(m_cropY, this);
            int cropSrcW = Widget::evalInt(m_cropW, this);
            int cropSrcH = Widget::evalInt(m_cropH, this);

            fflassert(cropSrcW >= 0, cropSrcW);
            fflassert(cropSrcH >= 0, cropSrcH);

            if(!mathf::rectangleOverlapRegion<int>(0, 0, m_gfxWidget->w(), m_gfxWidget->h(), cropSrcX, cropSrcY, cropSrcW, cropSrcH)){
                return;
            }

            //       w0  w1  w2
            //     +---+---+---+
            //  h0 |   |   |   |
            //     +---+---+---+
            //  h1 |   |   |   |
            //     +---+---+---+
            //  h2 |   |   |   |
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

            const GfxCropBoard topLeft     {{.getter = m_gfxWidget, .roi{x0, y0, w0, h0}}};
            const GfxCropBoard top         {{.getter = m_gfxWidget, .roi{x1, y0, w1, h0}}};
            const GfxCropBoard topRight    {{.getter = m_gfxWidget, .roi{x2, y0, w2, h0}}};
            const GfxCropBoard left        {{.getter = m_gfxWidget, .roi{x0, y1, w0, h1}}};
            const GfxCropBoard middle      {{.getter = m_gfxWidget, .roi{x1, y1, w1, h1}}};
            const GfxCropBoard right       {{.getter = m_gfxWidget, .roi{x2, y1, w2, h1}}};
            const GfxCropBoard bottomLeft  {{.getter = m_gfxWidget, .roi{x0, y2, w0, h2}}};
            const GfxCropBoard bottom      {{.getter = m_gfxWidget, .roi{x1, y2, w1, h2}}};
            const GfxCropBoard bottomRight {{.getter = m_gfxWidget, .roi{x2, y2, w2, h2}}};

            const GfxDupBoard    topDup{DIR_UPLEFT, 0, 0, w() - w0 - w2,            h0, &top    };
            const GfxDupBoard   leftDup{DIR_UPLEFT, 0, 0,            w0, h() - h0 - h2, &left   };
            const GfxDupBoard middleDup{DIR_UPLEFT, 0, 0, w() - w0 - w2, h() - h0 - h2, &middle };
            const GfxDupBoard  rightDup{DIR_UPLEFT, 0, 0,            w2, h() - h0 - h2, &right  };
            const GfxDupBoard bottomDup{DIR_UPLEFT, 0, 0, w() - w0 - w2,            h2, &bottom };

            for(const auto &[widgetPtr, offX, offY]: std::initializer_list<std::tuple<const Widget *, int, int>>
            {
                {static_cast<const Widget *>(&topLeft    ),        0,        0},
                {static_cast<const Widget *>(&topDup     ),       w0,        0},
                {static_cast<const Widget *>(&topRight   ), w() - w2,        0},

                {static_cast<const Widget *>(&  leftDup  ),        0,       h0},
                {static_cast<const Widget *>(&middleDup  ),       w0,       h0},
                {static_cast<const Widget *>(& rightDup  ), w() - w2,       h0},

                {static_cast<const Widget *>(&bottomLeft ),        0, h() - h2},
                {static_cast<const Widget *>(&bottomDup  ),       w0, h() - h2},
                {static_cast<const Widget *>(&bottomRight), w() - w2, h() - h2},
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

                            offX,
                            offY,
                            widgetPtr->w(),
                            widgetPtr->h())){
                    widgetPtr->draw({.x = drawDstX, .y = drawDstY, .ro{drawSrcX, drawSrcY, drawSrcW, drawSrcH}});
                }
            }
        }
};
