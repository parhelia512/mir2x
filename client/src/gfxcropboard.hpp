#pragma once
#include <variant>
#include "protocoldef.hpp"
#include "mathf.hpp"
#include "widget.hpp"

class GfxCropBoard: public Widget
{
    private:
        using VarGfxWidget = std::variant <const Widget *,
                             std::function<const Widget *(const Widget *)>>;

    private:
        VarGfxWidget m_gfxWidgetGetter;

    private:
        const Widget::VarOff m_gfxCropX;
        const Widget::VarOff m_gfxCropY;
        const Widget::VarOff m_gfxCropW;
        const Widget::VarOff m_gfxCropH;

    private:
        const std::array<Widget::VarOff, 4> m_margin;

    public:
        GfxCropBoard(
                Widget::VarDir argDir,
                Widget::VarOff argX,
                Widget::VarOff argY,

                VarGfxWidget argWidgetGetter,

                Widget::VarOff argGfxCropX, // crop on gfx widget
                Widget::VarOff argGfxCropY, // ...
                Widget::VarOff argGfxCropW, // crop width, don't use Widget::VarSizeOpt, support over-cropping
                Widget::VarOff argGfxCropH, // ...

                std::array<Widget::VarOff, 4> argMargin = {},

                Widget *argParent     = nullptr,
                bool    argAutoDelete = false)

            : Widget
              {{
                  .dir = std::move(argDir),
                  .x = std::move(argX),
                  .y = std::move(argY),
                  .parent
                  {
                      .widget = argParent,
                      .autoDelete = argAutoDelete,
                  }
              }}

            , m_gfxWidgetGetter(std::move(argWidgetGetter))

            , m_gfxCropX(std::move(argGfxCropX))
            , m_gfxCropY(std::move(argGfxCropY))
            , m_gfxCropW(std::move(argGfxCropW))
            , m_gfxCropH(std::move(argGfxCropH))

            , m_margin(std::move(argMargin))
        {
            // respect blank space by over-cropping
            // if cropped size bigger than gfx size, it's fill with blank

            setSize([this](const Widget *)
            {
                return gfxCropW() + margin(2) + margin(3);
            },

            [this](const Widget *)
            {
                return gfxCropH() + margin(0) + margin(1);
            });
        }

    public:
        void draw(Widget::ROIMap m) const override
        {
            if(!m.calibrate(this)){
                return;
            }

            const auto gfxPtr = gfxWidget();
            if(!gfxPtr){
                return;
            }

            const int brdCropXOrig = gfxCropX();
            const int brdCropYOrig = gfxCropY();

            int brdCropX = brdCropXOrig;
            int brdCropY = brdCropYOrig;
            int brdCropW = gfxCropW();
            int brdCropH = gfxCropH();

            if(!mathf::rectangleOverlapRegion<int>(0, 0, gfxPtr->w(), gfxPtr->h(), brdCropX, brdCropY, brdCropW, brdCropH)){
                return;
            }

            int drawDstX = m.x;
            int drawDstY = m.y;
            int drawSrcX = m.ro->x;
            int drawSrcY = m.ro->y;
            int drawSrcW = m.ro->w;
            int drawSrcH = m.ro->h;

            if(!mathf::cropChildROI(
                        &drawSrcX, &drawSrcY,
                        &drawSrcW, &drawSrcH,
                        &drawDstX, &drawDstY,

                        w(),
                        h(),

                        margin(2) + brdCropX - brdCropXOrig,
                        margin(0) + brdCropY - brdCropYOrig,

                        brdCropW,
                        brdCropH)){
                return;
            }

            gfxPtr->draw({.x = drawDstX, .y = drawDstY, .ro{drawSrcX + brdCropX, drawSrcY + brdCropY, drawSrcW, drawSrcH}});
        }

    public:
        const Widget *gfxWidget() const
        {
            return std::visit(VarDispatcher
            {
                [](const Widget *arg) -> const Widget *
                {
                    return arg;
                },

                [this](const std::function<const Widget *(const Widget *)> &arg) -> const Widget *
                {
                    return arg ? arg(this) : nullptr;
                }
            },
            m_gfxWidgetGetter);
        }

    public:
        int gfxCropX() const { return                  Widget::evalInt(m_gfxCropX, this) ; }
        int gfxCropY() const { return                  Widget::evalInt(m_gfxCropY, this) ; }
        int gfxCropW() const { return std::max<int>(0, Widget::evalInt(m_gfxCropW, this)); }
        int gfxCropH() const { return std::max<int>(0, Widget::evalInt(m_gfxCropH, this)); }

    public:
        int margin(int index) const
        {
            return std::max<int>(0, Widget::evalInt(m_margin[((index % 4) + 4) % 4], this));
        }
};
