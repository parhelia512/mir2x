#pragma once
#include <string>
#include <cstdint>
#include <SDL2/SDL.h>

#include "colorf.hpp"
#include "widget.hpp"
#include "imageboard.hpp"

class TextBoard: public Widget
{
    private:
        uint8_t m_font;
        uint8_t m_fontSize;
        uint8_t m_fontStyle;

    private:
        std::function<std::string(const Widget *)> m_textFunc;

    private:
        ImageBoard m_image;

    public:
        TextBoard(
                Widget::VarDir,
                Widget::VarInt,
                Widget::VarInt,

                std::function<std::string(const Widget *)>,

                uint8_t =  0,
                uint8_t = 10,
                uint8_t =  0,

                Widget::VarU32       = colorf::WHITE + colorf::A_SHF(0XFF),
                Widget::VarBlendMode = SDL_BLENDMODE_NONE,

                Widget * = nullptr,
                bool     = false);

    public:
        void setFont(uint8_t argFont)
        {
            m_font = argFont;
        }

        void setFontSize(uint8_t argFontSize)
        {
            m_fontSize = argFontSize;
        }

        void setFontStyle(uint8_t argFontStyle)
        {
            m_fontStyle = argFontStyle;
        }

        void setFontColor(Widget::VarU32 argColor)
        {
            m_image.setColor(std::move(argColor));
        }

        void setTextFunc(std::function<std::string(const Widget *)> argTextFunc)
        {
            m_textFunc = std::move(argTextFunc);
        }

    public:
        bool empty() const
        {
            return m_textFunc ? m_textFunc(this).empty() : true;
        }

        void drawEx(int dstX, int dstY, const Widget::ROIOpt &roi) const override
        {
            return m_image.drawEx(dstX, dstY, roi);
        }
};
