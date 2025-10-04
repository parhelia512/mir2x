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
                Widget::VarOff argX,
                Widget::VarOff argY,

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
              {
                  std::move(argDir),
                  std::move(argX),
                  std::move(argY),

                  {},
                  {},
                  {},

                  argParent,
                  argAutoDelete,
              }

            , m_textShadow
              {
                  DIR_UPLEFT,
                  std::max<int>(0, argXShadowOff),
                  std::max<int>(0, argYShadowOff),

                  std::move(argTextFunc),

                  argFont,
                  argFontSize,
                  argFontStyle,

                  std::move(argFontShadowColor),
                  SDL_BLENDMODE_NONE,

                  this,
                  false,
              }

            , m_text
              {
                  DIR_UPLEFT,
                  0,
                  0,

                  std::move(argTextFunc),

                  argFont,
                  argFontSize,
                  argFontStyle,

                  std::move(argFontColor),
                  SDL_BLENDMODE_NONE,

                  this,
                  false,
              }
        {}
};
