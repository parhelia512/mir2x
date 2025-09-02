#pragma once
#include <cstdint>
#include <functional>

#include "widget.hpp"
#include "acbutton.hpp"
#include "texslider.hpp"
#include "layoutboard.hpp"
#include "tritexbutton.hpp"
#include "cropviewboard.hpp"
#include "alphaonbutton.hpp"
#include "shapecropboard.hpp"
#include "gfxcropdupboard.hpp"
#include "cbface.hpp"

class ProcessRun;
class ControlBoard;
class CBMiddle: public Widget
{
    private:
        friend class ControlBoard;

    private:
        ProcessRun *m_processRun;

    private:
        LayoutBoard &m_logBoard;

    private:
        ShapeCropBoard m_bg;

    private:
        CBFace m_face;

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

                ProcessRun *,

                Widget * = nullptr,
                bool     = false);

    public:
        bool processEventDefault(const SDL_Event &, bool, int, int, const Widget::ROIOpt &) override;
};
