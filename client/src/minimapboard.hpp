#pragma once
#include "widget.hpp"
#include "imageboard.hpp"
#include "itemflex.hpp"
#include "tritexbutton.hpp"
#include "marginwrapper.hpp"
#include "shapecropboard.hpp"

class ProcessRun;
class MiniMapBoard: public Widget
{
    private:
        bool m_alphaOn  = false;
        bool m_extended = false;

    private:
        ProcessRun *m_processRun;

    private:
        ShapeCropBoard m_canvas;

    private:
        ImageBoard m_cornerUpLeft;
        ImageBoard m_cornerUpRight;
        ImageBoard m_cornerDownLeft;

    private:
        TritexButton m_buttonAlpha;
        TritexButton m_buttonExtend;

    private:
        ItemFlex m_buttonFlex;

    private:
        MarginWrapper m_mouseLoc;

    public:
        MiniMapBoard(ProcessRun *, Widget * = nullptr, bool = false);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        void setPLoc();
        void flipExtended();
        void flipMiniMapShow();

    public:
        SDL_Texture *getMiniMapTexture() const;

    private:
        void drawMiniMapTexture(int, int) const;

    private:
        int getFrameSize() const;

    private:
        std::tuple<int, int> mouseOnMapGLoc(int, int) const;

    private:
        void drawCanvas(int, int);
};
