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
        struct InitArgs final
        {
            ProcessRun *proc {};
            Widget::WADPair parent {};
        };

    private:
        bool m_alphaOn = false;
        bool m_extended = false;
        bool m_autoCenter = true;

    private:
        double m_zoomFactor = 1.0;

    private:
        const int m_defaultSize = 300;

    private:
        ProcessRun *m_processRun;

    private:
        ShapeCropBoard m_bg;
        ImageBoard m_mapImage;

    private:
        ShapeCropBoard m_canvas;

    private:
        ImageBoard m_cornerUpLeft;
        ImageBoard m_cornerUpRight;
        ImageBoard m_cornerDownLeft;

    private:
        TritexButton m_buttonAlpha;
        TritexButton m_buttonExtend;
        TritexButton m_buttonAutoCenter;

    private:
        ItemFlex m_buttonFlex;

    public:
        MiniMapBoard(MiniMapBoard::InitArgs);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        void setPLoc(){}
        void flipExtended();
        void flipAutoCenter();

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

    private:
        std::tuple<int, int> canvasPLocOnMap(int, int) const;
        std::tuple<int, int> mapGLocOnCanvas(int, int) const;
        std::tuple<int, int> mapGLocOnImage (int, int) const;
};
