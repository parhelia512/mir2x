#include <array>
#include "client.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "minimapboard.hpp"
#include "maprecord.hpp"
#include "processrun.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

MiniMapBoard::MiniMapBoard(ProcessRun *argProc, Widget *argParent, bool argAutoDelete)
    : Widget
      {
          DIR_UPLEFT,
          0,
          0,

          [this]{ return getFrameSize(); },
          [this]{ return getFrameSize(); },

          {},

          argParent,
          argAutoDelete,
      }

    , m_processRun(argProc)
    , m_canvas
      {
          DIR_UPLEFT,
          0,
          0,

          [this]{ return w(); },
          [this]{ return h(); },

          [this](int drawDstX, int drawDstY)
          {
              if(getMiniMapTexture()){
                  drawCanvas(drawDstX, drawDstY);
              }
          },

          this,
          false,
      }

    , m_cornerUpLeft
      {
          DIR_UPLEFT,
          0,
          0,

          {},
          {},

          []{ return g_progUseDB->retrieve(0X09000006); },

          false,
          false,
          0,

          colorf::WHITE_A255,
          SDL_BLENDMODE_BLEND,

          this,
          false,
      }

    , m_cornerUpRight
      {
          DIR_UPRIGHT,
          [this]{ return w() - 1; },
          0,

          {},
          {},

          []{ return g_progUseDB->retrieve(0X09000007); },

          false,
          false,
          0,

          colorf::WHITE_A255,
          SDL_BLENDMODE_BLEND,

          this,
          false,
      }

    , m_cornerDownLeft
      {
          DIR_DOWNLEFT,
          0,
          [this]{ return h() - 1; },

          {},
          {},

          []{ return g_progUseDB->retrieve(0X09000008); },

          false,
          false,
          0,

          colorf::WHITE_A255,
          SDL_BLENDMODE_BLEND,

          this,
          false,
      }

    , m_buttonAlpha
      {
          DIR_UPLEFT,
          0,
          0,

          {
              0X09000002,
              0X09000002,
              0X09000002,
          },

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
              m_alphaOn = !m_alphaOn;
              if(m_alphaOn){
                  m_buttonAlpha.setTexID({0X09000003, 0X09000003, 0X09000003});
              }
              else{
                  m_buttonAlpha.setTexID({0X09000002, 0X09000002, 0X09000002});
              }
          },

          0,
          0,
          0,
          0,

          false,
      }

    , m_buttonExtend
      {
          DIR_UPLEFT,
          0,
          0,

          {
              0X09000004,
              0X09000004,
              0X09000004,
          },

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
              if(getMiniMapTexture()){
                  flipExtended();
              }

              if(m_extended){
                  m_buttonExtend.setTexID({0X09000005, 0X09000005, 0X09000005});
              }
              else{
                  m_buttonExtend.setTexID({0X09000004, 0X09000004, 0X09000004});
              }
          },

          0,
          0,
          0,
          0,

          false,
      }

    , m_buttonFlex
      {
          DIR_DOWNRIGHT,
          [this]{ return w() - 1; },
          [this]{ return h() - 1; },

          {},

          true,
          1,

          {
              {&m_buttonAlpha , false},
              {&m_buttonExtend, false},
          },

          this,
          false,
      }

    , m_mouseLoc
      ({
          DIR_UPLEFT,
          0,
          0,

          new TextBoard
          {
              DIR_UPLEFT,
              0,
              0,

              []{ return "LOC"; },

              1,
              12,
              0,

              colorf::YELLOW_A255,
              SDL_BLENDMODE_BLEND,
          },

          true,

          { // margin
              2,
              2,
              2,
              2,
          },

          // [this](const Widget *self, int drawDstX, int drawDstY)
          [this](const Widget *, int, int)
          {
              // g_sdlDevice->fillRectangle((m_processRun->canMove(true, 0, onMapPX, onMapPY) ? colorf::BLACK : colorf::RED) + colorf::A_SHF(200), mousePX, mousePY, locBoard.w(), locBoard.h());
          }
      })
{
    setShow([this] -> bool { return getMiniMapTexture(); });
}

bool MiniMapBoard::processEventDefault(const SDL_Event &event, bool valid, int startDstX, int startDstY, const Widget::ROIOpt &roi)
{
    const auto roiOpt = cropDrawROI(startDstX, startDstY, roi);
    if(!roiOpt.has_value()){
        return false;
    }

    if(!valid){
        m_buttonAlpha .setOff();
        m_buttonExtend.setOff();
        return false;
    }

    bool took = false;
    took |= m_buttonAlpha .processEvent(event, valid && !took, startDstX, startDstY, roiOpt.value());
    took |= m_buttonExtend.processEvent(event, valid && !took, startDstX, startDstY, roiOpt.value());

    if(took){
        return true;
    }

    switch(event.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                if(event.button.button == SDL_BUTTON_RIGHT){
                    if(in(event.button.x, event.button.y, startDstX, startDstY, roiOpt.value())){
                        const auto remapXDiff = startDstX - roiOpt->x;
                        const auto remapYDiff = startDstY - roiOpt->y;
                        const auto [onMapPX, onMapPY] = mouseOnMapGLoc(event.button.x - remapXDiff, event.button.y - remapYDiff);
                        m_processRun->requestSpaceMove(std::get<0>(m_processRun->getMap()), onMapPX, onMapPY);
                        return true;
                    }
                }
                return false;
            }
        default:
            {
                return false;
            }
    }
}

void MiniMapBoard::flipExtended()
{
    m_extended = !m_extended;
    setPLoc();
    m_buttonAlpha .setOff();
    m_buttonExtend.setOff();
}

void MiniMapBoard::drawCanvas(int drawDstX, int drawDstY)
{
    auto mapTexPtr = getMiniMapTexture();
    if(!mapTexPtr){
        return;
    }

    if(!m_alphaOn){
        g_sdlDevice->fillRectangle(colorf::BLACK_A255, drawDstX, drawDstY, w(), h());
    }

}

void MiniMapBoard::drawMiniMapTexture(int drawDstX, int drawDstY) const
{
    auto mapTexPtr = getMiniMapTexture();
    if(!mapTexPtr){
        return;
    }

    const auto [mapUID, mapW, mapH] = m_processRun->getMap();
    const auto [texW, texH] = SDLDeviceHelper::getTextureSize(mapTexPtr);
    const auto fnGetMPLoc = [mapW, mapH, texW, texH](const std::tuple<int, int> &loc) -> std::tuple<int, int>
    {
        return
        {
            to_d(std::lround((std::get<0>(loc) * 1.0 / mapW) * texW)),
            to_d(std::lround((std::get<1>(loc) * 1.0 / mapH) * texH)),
        };
    };

    if(!m_alphaOn){
        g_sdlDevice->fillRectangle(colorf::BLACK + colorf::A_SHF(255), drawDstX, drawDstY, w(), h());
    }

    const auto [heroMPX, heroMPY] = fnGetMPLoc(m_processRun->getMyHero()->location());
    const int srcX = std::min<int>(std::max<int>(0, heroMPX - w() / 2), texW - w());
    const int srcY = std::min<int>(std::max<int>(0, heroMPY - h() / 2), texH - h());
    {
        SDLDeviceHelper::EnableTextureModColor enableModColor(mapTexPtr, colorf::WHITE + colorf::A_SHF(m_alphaOn ? 128 : 255));
        g_sdlDevice->drawTexture(mapTexPtr, drawDstX, drawDstY, srcX, srcY, w(), h());
    }

    for(const auto &p: m_processRun->getCOList()){
        const auto [coMPX, coMPY] = fnGetMPLoc(p.second->location());
        const auto [color, r] = [this](uint64_t uid) -> std::tuple<uint32_t, int>
        {
            switch(uidf::getUIDType(uid)){
                case UID_PLY:
                    {
                        if(uid == m_processRun->getMyHeroUID()){
                            return {colorf::RGBA(255, 0, 255, 255), 2};
                        }
                        else{
                            return {colorf::RGBA(200, 0, 200, 255), 2};
                        }
                    }
                case UID_NPC:
                    {
                        return {colorf::BLUE+ colorf::A_SHF(255), 2};
                    }
                case UID_MON:
                    {
                        return {colorf::RED + colorf::A_SHF(255), 1};
                    }
                default:
                    {
                        return {0, 0};
                    }
            }
        }(p.first);

        if(colorf::A(color)){
            g_sdlDevice->fillRectangle(color, drawDstX + (coMPX - srcX) - r, drawDstY + (coMPY - srcY) - r, 2 * r + 1, 2 * r + 1);
        }
    }

    g_sdlDevice->drawRectangle(colorf::RGBA(60, 60, 60, 255), drawDstX, drawDstY, w(), h());
    if(auto frameTexPtr = g_progUseDB->retrieve(0X09000006); frameTexPtr){
        g_sdlDevice->drawTexture(frameTexPtr, drawDstX, drawDstY);
    }

    if(auto frameTexPtr = g_progUseDB->retrieve(0X09000007); frameTexPtr){
        g_sdlDevice->drawTexture(frameTexPtr, drawDstX + w() - SDLDeviceHelper::getTextureWidth(frameTexPtr), drawDstY);
    }

    if(auto frameTexPtr = g_progUseDB->retrieve(0X09000008); frameTexPtr){
        g_sdlDevice->drawTexture(frameTexPtr, drawDstX, drawDstY + h() - SDLDeviceHelper::getTextureHeight(frameTexPtr));
    }

    const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc();
    if(in(mousePX, mousePY, drawDstX, drawDstY, roi())){
        const auto onMapPX = std::lround((mousePX - drawDstX + srcX) * 1.0 * mapW / texW);
        const auto onMapPY = std::lround((mousePY - drawDstY + srcY) * 1.0 * mapH / texH);

        const auto locStr = str_printf(u8"[%ld,%ld]", onMapPX, onMapPY);
        LabelBoard locBoard(DIR_DOWNRIGHT, mousePX, mousePY, locStr.c_str(), 1, 12, 0, colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF));
        g_sdlDevice->fillRectangle((m_processRun->canMove(true, 0, onMapPX, onMapPY) ? colorf::BLACK : colorf::RED) + colorf::A_SHF(200), mousePX, mousePY, locBoard.w(), locBoard.h());
        locBoard.drawRoot();
    }
}

int MiniMapBoard::getFrameSize() const
{
    const auto [texW, texH] = SDLDeviceHelper::getTextureSize(getMiniMapTexture());
    if(m_extended){
        return std::min<int>({texW, texH, 300}); // setup window size here
    }else{
        return std::min<int>({texW, texH, 128});
    }
}

SDL_Texture *MiniMapBoard::getMiniMapTexture() const
{
    if(const auto miniMapIDOpt = DBCOM_MAPRECORD(m_processRun->mapID()).miniMapID; miniMapIDOpt.has_value()){
        return g_progUseDB->retrieve(miniMapIDOpt.value());
    }
    return nullptr;
}

void MiniMapBoard::flipMiniMapShow()
{
    flipShow();
    setPLoc();
}

std::tuple<int, int> MiniMapBoard::mouseOnMapGLoc(int xOff, int yOff) const
{
    auto mapTexPtr = getMiniMapTexture();
    fflassert(mapTexPtr);

    const auto [mapUID, mapW, mapH] = m_processRun->getMap();
    const auto [texW, texH] = SDLDeviceHelper::getTextureSize(mapTexPtr);
    const auto fnGetMPLoc = [mapW, mapH, texW, texH](const std::tuple<int, int> &loc) -> std::tuple<int, int>
    {
        return
        {
            to_d(std::lround((std::get<0>(loc) * 1.0 / mapW) * texW)),
            to_d(std::lround((std::get<1>(loc) * 1.0 / mapH) * texH)),
        };
    };

    const auto [heroMPX, heroMPY] = fnGetMPLoc(m_processRun->getMyHero()->location());
    const int srcX = std::min<int>(std::max<int>(0, heroMPX - w() / 2), texW - w());
    const int srcY = std::min<int>(std::max<int>(0, heroMPY - h() / 2), texH - h());

    return
    {
        std::lround((xOff + srcX) * 1.0 * mapW / texW),
        std::lround((yOff + srcY) * 1.0 * mapH / texH),
    };
}

void MiniMapBoard::setPLoc()
{}
