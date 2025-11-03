#pragma once
#include <optional>
#include <functional>
#include "widget.hpp"

class ShapeCropBoard: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarSizeOpt w = 0;
            Widget::VarSizeOpt h = 0;

            Widget::VarDrawFunc drawFunc = nullptr;
            Widget::WADPair parent {};
        };

    private:
        Widget::VarDrawFunc m_drawFunc;

    public:
        ShapeCropBoard(ShapeCropBoard::InitArgs);

    public:
        void draw(Widget::ROIMap) const override;
};
