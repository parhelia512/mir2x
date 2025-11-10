#pragma once
#include <variant>
#include <functional>
#include "widget.hpp"

class CropViewBoard: public Widget
{
    private:
        using VarGfxWidget = std::variant <Widget *,
                             std::function<Widget *(const Widget *)>>;

    private:
        VarGfxWidget m_gfxWidgetGetter;

    private:
        const Widget::VarInt m_gfxCropX;
        const Widget::VarInt m_gfxCropY;
        const Widget::VarInt m_gfxCropW;
        const Widget::VarInt m_gfxCropH;

    private:
        const std::array<Widget::VarInt, 4> m_margin;

    public:
        CropViewBoard(
                Widget::VarDir,
                Widget::VarInt,
                Widget::VarInt,

                VarGfxWidget,

                Widget::VarInt, // crop on gfx widget
                Widget::VarInt, // ...
                Widget::VarInt, // crop width, don't use Widget::VarSizeOpt, support over-cropping
                Widget::VarInt, // ...

                std::array<Widget::VarInt, 4> = {},

                Widget * = nullptr,
                bool     = false);

    public:
        void drawDefault(Widget::ROIMap) const override;

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

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
