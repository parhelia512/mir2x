#pragma once
#include <array>
#include <cstdint>
#include <functional>
#include "buttonbase.hpp"

class TrigfxButton: public ButtonBase
{
    private:
        std::array<const Widget *, 3> m_gfxList;

    public:
        TrigfxButton(
                Widget::VarDir,
                Widget::VarInt,
                Widget::VarInt,

                std::array<const Widget *, 3>,
                std::array<std::optional<uint32_t>, 3>,

                std::function<void(Widget *      )> = nullptr,
                std::function<void(Widget *      )> = nullptr,
                std::function<void(Widget *, bool, int)> = nullptr,
                std::function<void(Widget *      , int)> = nullptr,

                int = 0,
                int = 0,
                int = 0,
                int = 0,

                bool = true,
                bool = false,

                Widget * = nullptr,
                bool     = false);

    public:
        void drawEx(int, int, const Widget::ROIOpt &) const override;

    private:
        void initButtonSize();

    public:
        void setGfxList(const std::array<const Widget *, 3> &gfxList)
        {
            m_gfxList = gfxList;
        }
};
