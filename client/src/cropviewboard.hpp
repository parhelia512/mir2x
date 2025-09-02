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
        const Widget::VarOff m_gfxCropX;
        const Widget::VarOff m_gfxCropY;
        const Widget::VarOff m_gfxCropW;
        const Widget::VarOff m_gfxCropH;

    private:
        const std::array<Widget::VarOff, 4> m_margin;

    public:
        CropViewBoard(
                Widget::VarDir,
                Widget::VarOff,
                Widget::VarOff,

                VarGfxWidget,

                Widget::VarOff, // crop on gfx widget
                Widget::VarOff, // ...
                Widget::VarOff, // crop width, don't use Widget::VarSize, support over-cropping
                Widget::VarOff, // ...

                std::array<Widget::VarOff, 4> = {},

                Widget * = nullptr,
                bool     = false);

    public:
        void drawEx(int, int, const Widget::ROIOpt &) const override;

    public:
        bool processEventDefault(const SDL_Event &, bool, int, int, const Widget::ROIOpt &) override;

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
