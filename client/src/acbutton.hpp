#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <functional>
#include <unordered_map>

#include "widget.hpp"
#include "buttonbase.hpp"
#include "labelboard.hpp"

class ProcessRun;
class ACButton: public ButtonBase
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;;
            Widget::VarOff x   = 0;
            Widget::VarOff y   = 0;

            ProcessRun *proc = nullptr;
            std::vector<std::string> names {};

            Widget::WADPair parent {};
        };

    private:
        ProcessRun *m_proc;
        const std::unordered_map<std::string, uint32_t> m_texMap;

    private:
        size_t m_currButtonName;
        const std::vector<std::string> m_buttonNameList;

    private:
        LabelBoard m_labelBoard;

    public:
        ACButton(ACButton::InitArgs);

    public:
        void draw(Widget::ROIMap) const override;

    private:
        void setLabel();
};
