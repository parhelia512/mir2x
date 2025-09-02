#pragma once
#include <string>
#include "serdesmsg.hpp"
#include "widget.hpp"

struct ChatPreviewPage: public Widget
{
    Widget canvas;
    ChatPreviewPage(Widget::VarDir,

            Widget::VarInt,
            Widget::VarInt,
            Widget::VarOptSize,
            Widget::VarOptSize,

            Widget * = nullptr,
            bool     = false);

    void updateChatPreview(const SDChatPeerID &, const std::string &);
};
