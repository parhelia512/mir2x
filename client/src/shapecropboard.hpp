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

            Widget::VarSize w = 0;
            Widget::VarSize h = 0;

            Widget::VarDrawFunc bgDrawFunc = nullptr;
            Widget::VarDrawFunc fgDrawFunc = nullptr;

            Widget::WADPair parent {};
        };

    private:
        Widget::VarDrawFunc m_bgDrawFunc;
        Widget::VarDrawFunc m_fgDrawFunc;

    public:
        ShapeCropBoard(ShapeCropBoard::InitArgs);

    public:
        void drawDefault(Widget::ROIMap) const override;
};
