#pragma once
#include <functional>
#include "colorf.hpp"
#include "widget.hpp"
#include "xmltypeset.hpp"
#include "ime.hpp"

class InputLine: public Widget
{
    protected:
        bool m_imeEnabled = true;

    protected:
        XMLTypeset m_tpset;

    protected:
        int m_cursor = 0;

    protected:
        int      m_cursorWidth;
        uint32_t m_cursorColor;
        double   m_cursorBlink = 0.0;

    protected:
        std::function<void()>            m_onTab;
        std::function<void()>            m_onCR;
        std::function<void(std::string)> m_onChange;

    public:
        InputLine(
                Widget::VarDir,
                Widget::VarInt,
                Widget::VarInt,

                Widget::VarOptSize,
                Widget::VarOptSize,

                bool,

                uint8_t  = 0,
                uint8_t  = 10,
                uint8_t  = 0,
                uint32_t = colorf::WHITE + colorf::A_SHF(255),

                int      = 2,
                uint32_t = colorf::WHITE + colorf::A_SHF(255),

                std::function<void()>            = nullptr,
                std::function<void()>            = nullptr,
                std::function<void(std::string)> = nullptr,

                Widget * = nullptr,
                bool     = false);

    public:
        bool processEventDefault(const SDL_Event &, bool, int, int, const Widget::ROIOpt &) override;

    public:
        void drawEx(int, int, const Widget::ROIOpt &) const override;

    public:
        void update(double ms) override
        {
            m_cursorBlink += ms;
        }

    public:
        std::string getRawString() const
        {
            return m_tpset.getRawString();
        }

    public:
        virtual void clear();

    public:
        void deleteChar();
        void insertChar(char);
        void setInput(const char *);
        void insertUTF8String(const char *);
};
