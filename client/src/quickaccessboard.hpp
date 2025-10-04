#pragma once
#include <tuple>
#include <cstdint>
#include "widget.hpp"
#include "tritexbutton.hpp"
#include "shapecropboard.hpp"

class ProcessRun;
class QuickAccessBoard: public Widget
{
    private:
        struct Grid: public Widget
        {
            int         slot;
            ProcessRun *proc;

            ShapeCropBoard bg;
            ImageBoard     item;
            TextBoard      count;

            Grid(   Widget::VarDir,
                    Widget::VarOff,
                    Widget::VarOff,

                    Widget::VarSizeOpt,
                    Widget::VarSizeOpt,

                    int,
                    ProcessRun *,

                    Widget * = nullptr,
                    bool     = false);
        };

    private:
        constexpr static uint32_t m_texID = 0X00000060;

    private:
        ProcessRun *m_processRun;

    private:
        ImageBoard m_bg;
        TritexButton m_buttonClose;

    public:
        QuickAccessBoard(dir8_t,
                int,
                int,

                ProcessRun *,

                Widget * = nullptr,
                bool     = false);

    public:
        bool processEventDefault(const SDL_Event &, bool, int, int, const Widget::ROIOpt &) override;

    public:
        static std::tuple<int, int, int, int> getGridLoc(int slot)
        {
            fflassert(slot >= 0, slot);
            fflassert(slot <  6, slot);

            return {17 + 42 * slot, 6, 36, 36};
        }

    public:
        void gridConsume(int);
};
