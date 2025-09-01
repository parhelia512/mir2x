#pragma once
#include <cstdint>
#include <optional>
#include "serdesmsg.hpp"
#include "widget.hpp"
#include "imageboard.hpp"
#include "shapecropboard.hpp"

class ProcessRun;
class CBFace: public Widget
{
    private:
        constexpr static int BAR_HEIGHT = 5;

    private:
        ProcessRun *m_processRun;

    private:
        ImageBoard m_face;
        ImageBoard m_hpBar;

    private:
        ShapeCropBoard m_drawBuffIDList;

    public:
        CBFace( Widget::VarDir,
                Widget::VarOff,
                Widget::VarOff,

                Widget::VarSize,
                Widget::VarSize,

                ProcessRun *,

                Widget * = nullptr,
                bool     = false);

    private:
        double getHPRatio() const;
        uint32_t getFaceTexID() const;
        const std::optional<SDBuffIDList> &getSDBuffIDListOpt() const;

    private:
        void drawBuffIDList(int, int, int, int) const;
};
