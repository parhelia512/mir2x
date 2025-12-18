#pragma once
#include <cstdint>
#include <vector>
#include <span>
#include "processrun.hpp"
#include "widget.hpp"
#include "imageboard.hpp"

class MagicIconButton;
class SkillBoardConfig;
class SkillPage: public Widget
{
    private:
        SkillBoardConfig * const m_config;
        ProcessRun       * const m_processRun;

    private:
        std::vector<MagicIconButton *> m_magicIconButtonList;

    private:
        ImageBoard m_bg;

    public:
        SkillPage(uint32_t, SkillBoardConfig *, ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void addIcon(uint32_t);

    public:
        const auto & getMagicIconButtonList() const
        {
            return m_magicIconButtonList;
        }
};
