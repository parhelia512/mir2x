#pragma once
#include "widget.hpp"
#include "imageboard.hpp"
#include "shapecropboard.hpp"

class ProcessRun;
class CBFace: public Widget
{
    private:
        ProcessRun *m_processRun;

    private:
        ImageBoard m_face;
        ImageBoard m_hpBar;

    private:
        ShapeCropBoard m_drawBuffIDList;
};
