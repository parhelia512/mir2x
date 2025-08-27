#pragma once
#include "widget.hpp"
#include "serdesmsg.hpp"
#include "texslider.hpp"
#include "imageboard.hpp"
#include "tritexbutton.hpp"
#include "shapecropboard.hpp"

class ProcessRun;
class HorseBoard: public Widget
{
    private:
        ProcessRun *m_processRun;

    private:
        ShapeCropBoard m_greyBg;
        ImageBoard     m_imageBg;

    private:
        TritexButton m_close;

    private:
        TritexButton m_up;
        TritexButton m_down;
        TritexButton m_hide;
        TritexButton m_show;

    public:
        HorseBoard(
                dir8_t,

                int,
                int,

                ProcessRun *,

                Widget * = nullptr,
                bool     = false);

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEventDefault(const SDL_Event &, bool, int, int) override;
};
