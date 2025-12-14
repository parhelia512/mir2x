#pragma once
#include <cstdint>
#include "widget.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class SkillBoardConfig;
class MagicIconButton: public Widget
{
    // +-+-----+
    // |A|     |
    // +-+     |
    // |       |
    // +-------+-+
    //         |1|
    //         +-+

    private:
        const uint32_t m_magicID;

    private:
        SkillBoardConfig * const m_config;
        ProcessRun       * const m_processRun;

    private:
        TritexButton m_icon;

    public:
        MagicIconButton(dir8_t, int, int, uint32_t, SkillBoardConfig *, ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void drawDefault(Widget::ROIMap) const override;

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        bool cursorOn() const
        {
            return m_icon.getState() != BEVENT_OFF;
        }

    public:
        uint32_t magicID() const
        {
            return m_magicID;
        }
};
