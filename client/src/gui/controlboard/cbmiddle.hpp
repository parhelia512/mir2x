#pragma once
#include <cstdint>
#include <functional>

#include "log.hpp"
#include "widget.hpp"
#include "pngtexdb.hpp"
#include "levelbox.hpp"
#include "acbutton.hpp"
#include "sdldevice.hpp"
#include "inputline.hpp"
#include "texslider.hpp"
#include "layoutboard.hpp"
#include "texaniboard.hpp"
#include "wmdaniboard.hpp"
#include "tritexbutton.hpp"
#include "alphaonbutton.hpp"
#include "cbleft.hpp"
#include "cbright.hpp"

class ProcessRun;
class ControlBoard;
class CBMiddle: public Widget
{
    private:
        friend class ControlBoard;

    private:
        ProcessRun *m_processRun;

    private:
        ShapeCropBoard m_bg;

    private:
        ImageBoard   m_bgImgFull;
        GfxCropBoard m_bgImg;

    private:
        TritexButton m_switchMode;

    private:
        TexSlider m_slider;

    private:
        InputLine m_cmdLine;

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
        ~CBMiddle() = default;

    private:
        void drawFocusFace() const;
        void drawMiddleExpand() const;
        void drawMiddleDefault() const;
        void drawLogBoardExpand() const;
        void drawLogBoardDefault() const;
        void drawInputGreyBackground() const;

    public:
        void update(double) override;
        bool processEventDefault(const SDL_Event &, bool, int, int) override;

    public:
        void inputLineDone();

    public:
        void addParLog(const char *);

    public:
        void addLog(int, const char *);

    public:
        bool CheckMyHeroMoved();

    private:
        void switchExpandMode();

    private:
        void setButtonLoc();

    public:
        void onWindowResize(int, int);

    private:
        int logBoardStartY() const;
};
