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
        constexpr static int LOG_WINDOW_HEIGHT = 83;
        constexpr static int CMD_WINDOW_HEIGHT = 17;

    private:
        friend class ControlBoard;

    private:
        ProcessRun *m_processRun;

    private:
        LayoutBoard &m_logBoard;
        LayoutBoard &m_cmdBoard;

    private:
        int m_cmdBoardCropX = 0;
        int m_cmdBoardCropY = 0;

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
        GfxCropBoard m_cmdView;

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

    private:
        void onCmdCR();
        void onCmdCursorMove();

    private:
        int getCmdWindowWidth() const { return w() - 456 + 343; }
        int getLogWindowWidth() const { return w() - 456 + 343; }
};
