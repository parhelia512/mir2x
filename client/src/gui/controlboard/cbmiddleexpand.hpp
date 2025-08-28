#include "widget.hpp"

class ProcessRun;
class CBMiddleExpand: public Widget
{
    private:
        ProcessRun *m_processRun;

    private:
        ShapeCropBoard m_bg;

    private:
        CropViewBoard m_logView;

    private:
        ImageBoard      m_bgImgFull;
        GfxCropDupBoard m_bgImg;

    private:
        TritexButton m_buttonSwitchMode;

    private:
        TritexButton m_buttonEmoji;
        TritexButton m_buttonMute;

    private:
        TexSlider m_slider;

    public:
};
