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
        VarDrawFunc m_drawFunc;

    public:
        ShapeCropBoard(
                Widget::VarDir,
                Widget::VarOff,
                Widget::VarOff,

                Widget::VarSizeOpt,
                Widget::VarSizeOpt,

                VarDrawFunc,

                Widget * = nullptr,
                bool     = false);

    public:
        void draw(Widget::ROIMap) const override;
};
