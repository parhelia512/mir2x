#pragma once
#include <functional>
#include "widget.hpp"
#include "textboard.hpp"

class TextShadowBoard: public Widget
{
    private:
        TextBoard m_textShadow;
        TextBoard m_text;

    public:
        TextShadowBoard(
                Widget::VarDir argDir,
                Widget::VarInt argX,
                Widget::VarInt argY,

                int argXShadowOff,
                int argYShadowOff,

                std::function<std::string(const Widget *)> argTextFunc,

                uint8_t argFont      = 0,
                uint8_t argFontSize  = 8,
                uint8_t argFontStyle = 0,

                Widget::VarU32 argFontColor       = colorf::WHITE_A255,
                Widget::VarU32 argFontShadowColor = colorf::BLACK + colorf::A_SHF(128),

                Widget *argParent     = nullptr,
                bool    argAutoDelete = false)

            : Widget
              {{
                  .dir = std::move(argDir),
                  .x = std::move(argX),
                  .y = std::move(argY),
                  .parent
                  {
                      .widget = argParent,
                      .autoDelete = argAutoDelete,
                  }
              }}

            , m_textShadow
              {{
                  .x = std::max<int>(0, argXShadowOff),
                  .y = std::max<int>(0, argYShadowOff),

                  .textFunc = std::move(argTextFunc),
                  .font
                  {
                      .id = argFont,
                      .size = argFontSize,
                      .style = argFontStyle,
                      .color = std::move(argFontShadowColor),
                  },

                  .blendMode = SDL_BLENDMODE_NONE,
                  .parent{this},
              }}

            , m_text
              {{
                  .textFunc = std::move(argTextFunc),
                  .font
                  {
                      .id = argFont,
                      .size = argFontSize,
                      .style = argFontStyle,
                      .color = std::move(argFontColor),
                  },

                  .blendMode = SDL_BLENDMODE_NONE,
                  .parent{this},
              }}
        {}
};
