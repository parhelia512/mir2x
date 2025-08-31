#pragma once
#include <cstdint>
#include <functional>
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "buttonbase.hpp"
#include "imageboard.hpp"

class AlphaOnButton: public ButtonBase
{
    private:
        const uint32_t m_modColor;

    private:
        const uint32_t m_texID;

    private:
        const int m_onOffX;
        const int m_onOffY;
        const int m_onRadius;

    private:
        ImageBoard m_on;
        ImageBoard m_down;

    public:
        AlphaOnButton(
                Widget::VarDir,
                Widget::VarOff,
                Widget::VarOff,

                int,
                int,
                int,

                uint32_t,
                uint32_t,

                std::function<void(Widget *           )> = nullptr,
                std::function<void(Widget *           )> = nullptr,
                std::function<void(Widget *, bool, int)> = nullptr,
                std::function<void(Widget *,       int)> = nullptr,

                bool = true,

                Widget * = nullptr,
                bool     = false);
};
