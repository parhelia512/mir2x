#pragma once
#include <optional>
#include <functional>
#include "widget.hpp"

class ShapeCropBoard: public Widget
{
    private:
        std::function<void(const Widget *, int, int)> m_drawFunc;

    public:
        ShapeCropBoard(
                Widget::VarDir,
                Widget::VarInt,
                Widget::VarInt,

                Widget::VarOptSize,
                Widget::VarOptSize,

                std::function<void(const Widget *, int, int)>,

                Widget * = nullptr,
                bool     = false);

    public:
        void drawEx(int, int, const Widget::ROIOpt &) const override;
};
