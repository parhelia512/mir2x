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
      {{
          .w = [this]{ return getFrameSize(); },
          .h = [this]{ return getFrameSize(); },

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_processRun(argProc)
    , m_canvas
      {{
          .w = [this]{ return w(); },
          .h = [this]{ return h(); },

          .drawFunc = [this](int drawDstX, int drawDstY)
          {
              if(getMiniMapTexture()){
                  drawCanvas(drawDstX, drawDstY);
              }
          },

          .parent{this},
      }}

    , m_cornerUpLeft
      {{
          .texLoadFunc = []{ return g_progUseDB->retrieve(0X09000006); },

          .blendMode = SDL_BLENDMODE_BLEND,
          .parent{this},
      }}

    , m_cornerUpRight
      {{
          .dir = DIR_UPRIGHT,

          .x = [this]{ return w() - 1; },
          .y = 0,

          .texLoadFunc = []{ return g_progUseDB->retrieve(0X09000007); },
          .parent{this},
      }}

    , m_cornerDownLeft
      {{
          .dir = DIR_DOWNLEFT,

          .x = 0,
          .y = [this]{ return h() - 1; },

          .texLoadFunc = []{ return g_progUseDB->retrieve(0X09000008); },
          .parent{this},
      }}

    , m_buttonAlpha
      {{
          .texIDList
          {
              .off  = 0X09000002,
              .on   = 0X09000002,
              .down = 0X09000002,
          },

          .onTrigger = [this](Widget *, int)
          {
              m_alphaOn = !m_alphaOn;
              if(m_alphaOn){
                  m_buttonAlpha.setTexID({0X09000003, 0X09000003, 0X09000003});
              }
              else{
                  m_buttonAlpha.setTexID({0X09000002, 0X09000002, 0X09000002});
              }
          },
      }}

    , m_buttonExtend
      {{
          .texIDList
          {
              .off  = 0X09000004,
              .on   = 0X09000004,
              .down = 0X09000004,
          },

          .onTrigger = [this](Widget *, int)
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
      }}

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
          {{
              .textFunc = "LOC",
              .font
              {
                  .id = 1,
                  .size = 12,
                  .color = colorf::YELLOW_A255,
              },
          }},

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

bool MiniMapBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.crop(roi())){
        return false;
    }

    if(!valid){
        m_buttonAlpha .setOff();
        m_buttonExtend.setOff();
        return false;
    }

    bool took = false;
    took |= m_buttonAlpha .processEvent(event, valid && !took, m);
    took |= m_buttonExtend.processEvent(event, valid && !took, m);

    if(took){
        return true;
    }

    switch(event.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                if(event.button.button == SDL_BUTTON_RIGHT){
                    if(m.in(event.button.x, event.button.y)){
                        const auto remapXDiff = m.x - m.ro->x;
                        const auto remapYDiff = m.y - m.ro->y;
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
    if(Widget::ROIMap{.x=drawDstX, .y=drawDstY, .ro{roi()}}.in(mousePX, mousePY)){
        const auto onMapPX = std::lround((mousePX - drawDstX + srcX) * 1.0 * mapW / texW);
        const auto onMapPY = std::lround((mousePY - drawDstY + srcY) * 1.0 * mapH / texH);

        const auto locStr = str_printf(u8"[%ld,%ld]", onMapPX, onMapPY);
        LabelBoard locBoard
        {{
            .dir = DIR_DOWNRIGHT,
            .x = mousePX,
            .y = mousePY,

            .label = locStr.c_str(),
            .font
            {
                .id = 1,
                .size = 12,
                .color = colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
            },
        }};
        g_sdlDevice->fillRectangle((m_processRun->canMove(true, 0, onMapPX, onMapPY) ? colorf::BLACK : colorf::RED) + colorf::A_SHF(200), mousePX, mousePY, locBoard.w(), locBoard.h());
        locBoard.drawRoot({});
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
