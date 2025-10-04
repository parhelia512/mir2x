#pragma once
#include <cstdint>
#include "widget.hpp"
#include "imageboard.hpp"
#include "layoutboard.hpp"
#include "tritexbutton.hpp"
#include "gfxcropdupboard.hpp"

class ProcessRun;
class NPCChatBoard: public Widget
{
    private:
        const int m_margin;

    private:
        uint64_t m_npcUID = 0;
        std::string m_eventPath;

    private:
        ProcessRun *m_processRun;

    private:
        ImageBoard m_face;
        LayoutBoard m_chatBoard;
        TritexButton m_buttonClose;

    public:
        NPCChatBoard(
                Widget::VarDir,
                Widget::VarOff,
                Widget::VarOff,

                ProcessRun *,

                Widget * = nullptr,
                bool     = false);

    public:
        void loadXML(uint64_t, const char *, const char *);

    private:
        void onClickEvent(const char *, const char *, const char *, bool);

    private:
        uint32_t getNPCFaceKey() const
        {
            if(uidf::isNPChar(m_npcUID)){
                return 0X50000000 | uidf::getNPCID(m_npcUID);
            }
            else{
                return SYS_U32NIL;
            }
        }
};
