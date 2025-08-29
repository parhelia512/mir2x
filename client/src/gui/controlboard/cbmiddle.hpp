#pragma once
#include <cstdint>
#include <functional>

#include "widget.hpp"
#include "acbutton.hpp"
#include "texslider.hpp"
#include "layoutboard.hpp"
#include "tritexbutton.hpp"
#include "alphaonbutton.hpp"
#include "shapecropboard.hpp"
#include "cbface.hpp"

class ProcessRun;
class CBMiddle: public Widget
{
    private:
        ProcessRun *m_processRun;

    private:
        ShapeCropBoard m_bg;

    private:
        FocusedFace m_face;

    private:
        ImageBoard      m_bgImgFull;
        GfxCropDupBoard m_bgImg;

    private:
        TritexButton m_switchMode;

    private:
        TexSlider m_slider;

    private:
        CropViewBoard m_logView;

    public:
        CBMiddle(
                Widget::VarDir,
                Widget::VarOff,
                Widget::VarOff,

                Widget::VarSize,
                Widget::VarSize,

                ProcessRun *,

                Widget * = nullptr,
                bool     = false);

    public:
        void update(double) override;
        bool processEventDefault(const SDL_Event &, bool, int, int) override;
};
