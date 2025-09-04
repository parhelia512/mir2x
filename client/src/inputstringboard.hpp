#pragma once
#include <vector>
#include <cstdint>
#include "widget.hpp"
#include "imageboard.hpp"
#include "passwordbox.hpp"
#include "layoutboard.hpp"
#include "tritexbutton.hpp"
#include "shapecropboard.hpp"

class InputStringBoard: public Widget
{
    private:
        std::function<void(std::u8string)> m_onDone;

    private:
        ImageBoard m_bg;

    private:
        LayoutBoard m_textInfo;

    private:
        ShapeCropBoard m_inputBg;
        PasswordBox    m_input;

    private:
        TritexButton m_yesButton;
        TritexButton m_nopButton;

    public:
        InputStringBoard(
                Widget::VarDir,
                Widget::VarInt,
                Widget::VarInt,

                bool,

                Widget * = nullptr,
                bool     = false);

    public:
        void drawEx(int, int, const Widget::ROIOpt &) const override;

    private:
        void inputLineDone();

    public:
        void clear()
        {
            m_input.clear();
        }

    public:
        void waitInput(std::u8string, std::function<void(std::u8string)>);

    public:
        void setSecurity(bool security)
        {
            m_input.setSecurity(security);
        }
};
