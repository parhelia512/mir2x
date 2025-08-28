#include "widget.hpp"
#include "levelbox.hpp"

class ProcessRun;
class CBTitle: public Widget
{
    private:
        ProcessRun *m_processRun;

    private:
        ImageBoard m_bg;
        TexAniBoard m_arcAni;

    private:
        CBLevel m_level;
};
