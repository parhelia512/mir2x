#pragma once
#include <cstdint>
#include <functional>

#include "widget.hpp"
#include "acbutton.hpp"
#include "texslider.hpp"
#include "layoutboard.hpp"
#include "tritexbutton.hpp"
#include "gfxcropboard.hpp"
#include "alphaonbutton.hpp"
#include "gfxshapeboard.hpp"
#include "gfxresizeboard.hpp"
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
        GfxShapeBoard m_bg;

    private:
        CBFace m_face;

    private:
        ImageBoard     m_bgImgFull;
        GfxResizeBoard m_bgImg;

    private:
        TritexButton m_switchMode;

    private:
        TexSlider m_slider;

    private:
        GfxCropBoard m_logView;

    public:
        CBMiddle(
                Widget::VarDir,
                Widget::VarInt,
                Widget::VarInt,
                Widget::VarSizeOpt,

                ProcessRun *,

                Widget * = nullptr,
                bool     = false);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;
};
