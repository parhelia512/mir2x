#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <functional>

#include "colorf.hpp"
#include "totype.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "controlboard.hpp"
#include "cbright.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

CBRight::CBRight(
        Widget::VarDir argDir,
        Widget::VarInt argX,
        Widget::VarInt argY,

        ProcessRun *argProc,
        Widget     *argParent,
        bool        argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),

          166,
          133,

          {},
          argParent,
          argAutoDelete,
      }

    , m_processRun(argProc)

    , m_bgFull
      {
          DIR_UPLEFT,
          0,
          0,

          {},
          {},

          [](const Widget *)
          {
              return g_progUseDB->retrieve(0X00000012);
          },
      }

    , m_bg
      {
          DIR_UPLEFT,
          0,
          0,

          &m_bgFull,

          [this](const Widget *){ return m_bgFull.w() - w(); },
          0,
          [this](const Widget *){ return w(); },
          [this](const Widget *){ return h(); },

          {},

          this,
          false,
      }

    , m_buttonExchange
      {
          DIR_UPLEFT,
          4,
          6,

          1,
          1,
          10,

          colorf::WHITE + colorf::A_SHF(80),
          0X00000042,

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int)
          {
              if(auto cb = hasParent<ControlBoard>()){
                  cb->addLog(0, "exchange doesn't implemented yet");
              }
          },

          true,
          this,
      }

    , m_buttonMiniMap
      {
          DIR_UPLEFT,
          4,
          40,

          1,
          1,
          10,

          colorf::WHITE + colorf::A_SHF(80),
          0X00000043,

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int)
          {
              if(auto p = dynamic_cast<MiniMapBoard *>(m_processRun->getWidget("MiniMapBoard"))){
                  if(p->getMiniMapTexture()){
                      p->flipMiniMapShow();
                  }
                  else{
                      if(auto cb = hasParent<ControlBoard>()){
                          cb->addLog(CBLOG_ERR, to_cstr(u8"没有可用的地图"));
                      }
                  }
              }
          },

          true,
          this,
      }

    , m_buttonMagicKey
      {
          DIR_UPLEFT,
          4,
          75,

          1,
          1,
          10,

          colorf::WHITE + colorf::A_SHF(80),
          0X00000044,

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int)
          {
              m_processRun->flipDrawMagicKey();
          },

          true,
          this,
      }

    , m_buttonInventory
      {
          DIR_UPLEFT,
          48,
          33,
          {0X00000030, 0X00000030, 0X00000031},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int)
          {
              if(auto p = m_processRun->getWidget("InventoryBoard")){
                  p->flipShow();
              }
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          this,
          false,
      }

    , m_buttonHeroState
      {
          DIR_UPLEFT,
          77,
          31,
          {0X00000033, 0X00000033, 0X00000032},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int)
          {
              if(auto p = m_processRun->getWidget("PlayerStateBoard")){
                  p->flipShow();
              }
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          this,
          false,
      }

    , m_buttonHeroMagic
      {
          DIR_UPLEFT,
          105,
          33,
          {0X00000035, 0X00000035, 0X00000034},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int)
          {
              if(auto p = m_processRun->getWidget("SkillBoard")){
                  p->flipShow();
              }
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          this,
          false,
      }

    , m_buttonGuild
      {
          DIR_UPLEFT,
          40,
          11,
          {0X00000036, 0X00000036, 0X00000037},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int)
          {
              if(auto p = m_processRun->getWidget("GuildBoard")){
                  p->flipShow();
              }
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          this,
          false,
      }

    , m_buttonTeam
      {
          DIR_UPLEFT,
          72,
          8,
          {0X00000038, 0X00000038, 0X00000039},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int)
          {
              auto boardPtr = dynamic_cast<TeamStateBoard *>(m_processRun->getWidget("TeamStateBoard"));
              auto  heroPtr = m_processRun->getMyHero();

              if(heroPtr->hasTeam()){
                  boardPtr->flipShow();
                  if(boardPtr->show()){
                      boardPtr->refresh();
                  }
              }
              else{
                  m_processRun->setCursor(ProcessRun::CURSOR_TEAMFLAG);
              }
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          this,
          false,
      }

    , m_buttonQuest
      {
          DIR_UPLEFT,
          108,
          11,
          {0X0000003A, 0X0000003A, 0X0000003B},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int)
          {
              if(auto p = m_processRun->getWidget("QuestStateBoard")){
                  p->flipShow();
              }

              m_buttonQuest.stopBlink();
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          this,
          false,
      }

    , m_buttonHorse
      {
          DIR_UPLEFT,
          40,
          61,
          {0X0000003C, 0X0000003C, 0X0000003D},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int)
          {
              if(auto p = m_processRun->getWidget("HorseBoard")){
                  p->flipShow();
              }
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          this,
          false,
      }

    , m_buttonRuntimeConfig
      {
          DIR_UPLEFT,
          72,
          72,
          {0X0000003E, 0X0000003E, 0X0000003F},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int)
          {
              if(auto p = m_processRun->getWidget("RuntimeConfigBoard")){
                  p->flipShow();
              }
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          this,
          false,
      }

    , m_buttonFriendChat
      {
          DIR_UPLEFT,
          108,
          61,
          {0X00000040, 0X00000040, 0X00000041},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int)
          {
              if(auto p = m_processRun->getWidget("FriendChatBoard")){
                  p->flipShow();
              }
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          this,
          false,
      }

    , m_buttonAC
      {
          DIR_UPLEFT,
          1,
          105,

          argProc,
          {
              "AC",
              "MA",
          },

          this,
          false,
      }

    , m_buttonDC
      {
          DIR_UPLEFT,
          84,
          105,

          argProc,
          {
              "DC",
              "MC",
          },

          this,
          false,
      }
{}
