#pragma once
#include <optional>
#include <functional>
#include "widget.hpp"

class ShapeCropBoard: public Widget
{
    private:
        using VarDrawFunc = std::variant<std::function<void(                int, int)>,
                                         std::function<void(const Widget *, int, int)>>;

    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarOff x = 0;
            Widget::VarOff y = 0;

            Widget::VarSizeOpt w = 0;
            Widget::VarSizeOpt h = 0;

            VarDrawFunc drawFunc {};
            Widget::WADPair parent {};
        };

    private:
        VarDrawFunc m_drawFunc;

    public:
        ShapeCropBoard(ShapeCropBoard::InitArgs);

    public:
        void draw(Widget::ROIMap) const override;
};
