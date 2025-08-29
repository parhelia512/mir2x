#pragma once
#include <variant>
#include <functional>
#include "mathf.hpp"
#include "widget.hpp"

class CropViewBoard: public Widget
{
    private:
        using VarGfxWidget = std::variant <Widget *,
                             std::function<Widget *(const Widget *)>>;

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
        CropViewBoard(
                Widget::VarDir argDir,
                Widget::VarOff argX,
                Widget::VarOff argY,

                VarGfxWidget argWidgetGetter,

                Widget::VarOff argGfxCropX, // crop on gfx widget
                Widget::VarOff argGfxCropY, // ...
                Widget::VarOff argGfxCropW, // crop width, don't use Widget::VarSize, support over-cropping
                Widget::VarOff argGfxCropH, // ...

                std::array<Widget::VarOff, 4> argMargin = {},

                Widget *argParent     = nullptr,
                bool    argAutoDelete = false)

            : Widget
              {
                  std::move(argDir),
                  std::move(argX),
                  std::move(argY),
                  0,
                  0,

                  {},

                  argParent,
                  argAutoDelete,
              }

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
        void drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const override
        {
            if(!show()){
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

            int drawDstX = dstX;
            int drawDstY = dstY;
            int drawSrcX = srcX;
            int drawSrcY = srcY;
            int drawSrcW = srcW;
            int drawSrcH = srcH;

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

            gfxPtr->drawEx(drawDstX, drawDstY, drawSrcX + brdCropX, drawSrcY + brdCropY, drawSrcW, drawSrcH);
        }

        bool processEventDefault(const SDL_Event &event, bool valid, int startDstX, int startDstY) override
        {
            if(!show()){
                return false;
            }

            auto gfxPtr = gfxWidget();
            if(!gfxPtr){
                return false;
            }

            switch(event.type){
                case SDL_MOUSEBUTTONUP:
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEMOTION:
                    {
                        const auto [locX, locY] = SDLDeviceHelper::getEventPLoc(event).value();
                    }
                default:
                    {

                    }
            }
        }

    public:
        Widget *gfxWidget()
        {
            return std::visit(VarDispatcher
            {
                [](Widget *arg) -> Widget *
                {
                    return arg;
                },

                [this](std::function<Widget *(const Widget *)> &arg) -> Widget *
                {
                    return arg ? arg(this) : nullptr;
                }
            },
            m_gfxWidgetGetter);
        }

        const Widget *gfxWidget() const
        {
            return const_cast<CropViewBoard *>(this)->gfxWidget();
        }

    public:
        int gfxCropX() const { return                  Widget::evalOff(m_gfxCropX, this) ; }
        int gfxCropY() const { return                  Widget::evalOff(m_gfxCropY, this) ; }
        int gfxCropW() const { return std::max<int>(0, Widget::evalOff(m_gfxCropW, this)); }
        int gfxCropH() const { return std::max<int>(0, Widget::evalOff(m_gfxCropH, this)); }

    public:
        int margin(int index) const
        {
            return std::max<int>(0, Widget::evalOff(m_margin[((index % 4) + 4) % 4], this));
        }
};
