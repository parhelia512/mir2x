#pragma once
#include "widget.hpp"
#include "shapecropboard.hpp"
#include "cropviewboard.hpp"
#include "imageboard.hpp"
#include "gfxresizeboard.hpp"
#include "tritexbutton.hpp"
#include "texslider.hpp"
#include "layoutboard.hpp"

class ProcessRun;
class ControlBoard;
class CBMiddleExpand: public Widget
{
    private:
        friend class ControlBoard;

    private:
        ProcessRun *m_processRun;
        LayoutBoard &m_logBoard;

    private:
        ShapeCropBoard m_bg;

    private:
        CropViewBoard m_logView;

    private:
        ImageBoard     m_bgImgFull;
        GfxResizeBoard m_bgImg;

    private:
        TritexButton m_buttonSwitchMode;

    private:
        TritexButton m_buttonEmoji;
        TritexButton m_buttonMute;

    private:
        TexSlider m_slider;

    public:
        CBMiddleExpand(
                Widget::VarDir,
                Widget::VarInt,
                Widget::VarInt,

                Widget::VarSizeOpt,

                ProcessRun *,

                Widget * = nullptr,
                bool     = false);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap);
};
