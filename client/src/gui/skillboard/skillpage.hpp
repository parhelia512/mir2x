#pragma once
#include <cstdint>
#include <vector>
#include "widget.hpp"
#include "processrun.hpp"

class MagicIconButton;
class SkillBoardConfig;
class SkillPage: public Widget
{
    private:
        SkillBoardConfig * const m_config;
        ProcessRun       * const m_processRun;

    private:
        const uint32_t m_pageImage;
        std::vector<MagicIconButton *> m_magicIconButtonList;

    public:
        SkillPage(uint32_t, SkillBoardConfig *, ProcessRun *proc, Widget *widgetPtr = nullptr, bool autoDelete = false);

    public:
        void addIcon(uint32_t);

    public:
        void drawDefault(Widget::ROIMap) const override;

    public:
        const auto &getMagicIconButtonList() const
        {
            return m_magicIconButtonList;
        }
};
