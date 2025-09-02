#pragma once
#include "widget.hpp"
#include "layoutboard.hpp"

struct ChatInputContainer: public Widget
{
    LayoutBoard layout;
    ChatInputContainer(
            Widget::VarDir,
            Widget::VarInt,
            Widget::VarInt,
            Widget::VarSize, // width only

            Widget * = nullptr,
            bool     = false);
};
