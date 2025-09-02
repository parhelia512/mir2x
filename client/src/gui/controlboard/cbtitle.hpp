#include "widget.hpp"
#include "imageboard.hpp"
#include "texaniboard.hpp"
#include "cblevel.hpp"

//                     |
//                     v
//       +-----+      ---
//      /       \      19
//  +--/  TITLE  \--+ ---
//  | /           \ |  ^
//  +---------------+  |

class ProcessRun;
class CBTitle: public Widget
{
    public:
        constexpr static int UP_HEIGHT = 19;

    private:
        ProcessRun *m_processRun;

    private:
        ImageBoard m_bg;
        TexAniBoard m_arcAni;

    private:
        CBLevel m_level;

    public:
        CBTitle(
                Widget::VarDir,
                Widget::VarInt,
                Widget::VarInt,

                ProcessRun *,

                Widget * = nullptr,
                bool    = true);
};
