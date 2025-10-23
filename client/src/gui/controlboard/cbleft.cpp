#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <functional>

#include "colorf.hpp"
#include "totype.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "cbleft.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

CBLeft::CBLeft(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        ProcessRun *argProc,
        Widget *argParent,
        bool argAutoDelete)

    : Widget
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),
          .w = 178,
          .h = 133,

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_processRun(argProc)

    , m_bgFull
      {{
          .texLoadFunc = [](const Widget *)
          {
              return g_progUseDB->retrieve(0X00000012);
          },
      }}

    , m_bg
      {
          DIR_UPLEFT,
          0,
          0,

          &m_bgFull,

          0,
          0,
          [this](const Widget *){ return w(); },
          [this](const Widget *){ return h(); },

          {},

          this,
          false,
      }

    , m_hpFull
      {{
          .texLoadFunc = [](const Widget *)
          {
              return g_progUseDB->retrieve(0X00000018);
          },
      }}

    , m_hp
      {
          DIR_DOWNLEFT,
          33,
          95,

          &m_hpFull,

          0,
          [this](const Widget *)
          {
              return m_hpFull.h() - m_hp.gfxCropH();
          },

          m_hpFull.w(),
          [this](const Widget *) -> int
          {
              if(auto myHero = m_processRun->getMyHero()){
                  return to_dround(m_hpFull.h() * myHero->getHealthRatio().at(0));
              }
              return 0;
          },

          {},

          this,
          false,
      }

    , m_mpFull
      {{
          .texLoadFunc = []
          {
              return g_progUseDB->retrieve(0X00000019);
          },
      }}

    , m_mp
      {
          DIR_DOWNLEFT,
          73,
          95,

          &m_mpFull,

          0,
          [this](const Widget *)
          {
              return m_mpFull.h() - m_mp.gfxCropH();
          },

          m_mpFull.w(),
          [this](const Widget *) -> int
          {
              if(auto myHero = m_processRun->getMyHero()){
                  return to_dround(m_mpFull.h() * myHero->getHealthRatio().at(1));
              }
              return 0;
          },

          {},

          this,
          false,
      }

    , m_levelBarFull
      {{
          .texLoadFunc = []
          {
              return g_progUseDB->retrieve(0X000000A0);
          },
      }}

    , m_levelBar
      {
          DIR_DOWN,
          153,
          115,

          &m_levelBarFull,

          0,
          [this](const Widget *)
          {
              return m_levelBarFull.h() - m_levelBar.gfxCropH();
          },

          m_levelBarFull.w(),
          [this](const Widget *) -> int
          {
              if(auto myHero = m_processRun->getMyHero()){
                  return to_dround(m_levelBarFull.h() * myHero->getLevelRatio());
              }
              return 0;
          },

          {},

          this,
          false,
      }

    , m_inventoryBarFull
      {{
          .texLoadFunc = [](const Widget *)
          {
              return g_progUseDB->retrieve(0X000000A0);
          },
      }}

    , m_inventoryBar
      {
          DIR_DOWN,
          166,
          115,

          &m_inventoryBarFull,

          0,
          [this](const Widget *)
          {
              return m_inventoryBarFull.h() - m_inventoryBar.gfxCropH();
          },

          m_inventoryBarFull.w(),
          [this](const Widget *) -> int
          {
              if(auto myHero = m_processRun->getMyHero()){
                  return to_dround(m_inventoryBarFull.h() * myHero->getInventoryRatio());
              }
              return 0;
          },

          {},

          this,
          false,
      }

    , m_buttonQuickAccess
      {{
          .x = 148,
          .y = 2,

          .texIDList
          {
              .on   = 0X0B000000,
              .down = 0X0B000001,
          },

          .onTrigger = [this](Widget *, int)
          {
              if(auto p = m_processRun->getWidget("QuickAccessBoard")){
                  p->flipShow();
              }
          },

          .parent{this},
      }}

    , m_buttonClose
      {{
          .x = 8,
          .y = 72,

          .texIDList
          {
              .on   = 0X0000001E,
              .down = 0X0000001F,
          },

          .onTrigger = [](Widget *, int)
          {
              std::exit(0);
          },

          .parent{this},
      }}

    , m_buttonMinize
      {{
          .x = 109,
          .y = 72,

          .texIDList
          {
              .on   = 0X00000020,
              .down = 0X00000021,
          },

          .parent{this},
      }}

    , m_mapGLocFull
      {
          DIR_UPLEFT,
          0,
          0,

          [this](const Widget *)
          {
              return getMapGLocStr();
          },

          1,
          12,
          0,

          colorf::WHITE_A255,
      }

    , m_mapGLoc
      {
          DIR_NONE,
          73,
          117,

          &m_mapGLocFull,

          [this](const Widget *)
          {
              if(m_mapGLocFull.w() < m_mapGLocMaxWidth){
                  return 0;
              }
              return to_d(m_mapGLocPixelSpeed * m_mapGLocAccuTime / 1000.0) % (m_mapGLocFull.w() - m_mapGLocMaxWidth);
          },
          0,

          [this](const Widget *)
          {
              return std::min<int>(m_mapGLocFull.w(), m_mapGLocMaxWidth);
          },

          [this](const Widget *)
          {
              return m_mapGLocFull.h();
          },

          {},

          this,
          false,
      }
{}

std::string CBLeft::getMapGLocStr() const
{
    if(uidf::isMap(m_processRun->mapUID())){
        if(const auto &mr = DBCOM_MAPRECORD(m_processRun->mapID())){
            const auto mapNameFull = std::string(to_cstr(mr.name));
            const auto mapNameBase = mapNameFull.substr(0, mapNameFull.find('_'));

            if(auto myHero = m_processRun->getMyHero()){
                return str_printf("%s: %d %d", mapNameBase.c_str(), myHero->x(), myHero->y());
            }
            else{
                return str_printf("%s", mapNameBase.c_str());
            }
        }
    }
    return {};
}
