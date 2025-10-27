#include <array>
#include "client.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "textboard.hpp"
#include "minimapboard.hpp"
#include "marginwrapper.hpp"
#include "maprecord.hpp"
#include "processrun.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

MiniMapBoard::MiniMapBoard(MiniMapBoard::InitArgs args)
    : Widget
      {{
          .dir = [this]
          {
              return m_extended ? DIR_NONE : DIR_UPRIGHT;
          },

          .x = [this]
          {
              if(m_extended){
                  return g_sdlDevice->getRendererWidth() / 2;
              }
              else{
                  return g_sdlDevice->getRendererWidth() - 1;
              }
          },

          .y = [this]
          {
              if(m_extended){
                  return g_sdlDevice->getRendererHeight() / 2;
              }
              else{
                  return 0;
              }
          },

          .w = [this]{ return m_extended ? to_dround(g_sdlDevice->getRendererWidth () * 0.8) : 200; },
          .h = [this]{ return m_extended ? to_dround(g_sdlDevice->getRendererHeight() * 0.8) : 200; },

          .parent = std::move(args.parent),
      }}

    , m_processRun(args.proc)
    , m_bg
      {{
          .w = [this]{ return w(); },
          .h = [this]{ return h(); },

          .drawFunc = [this](int drawDstX, int drawDstY)
          {
              g_sdlDevice->fillRectangle(colorf::BLACK_A255, drawDstX, drawDstY, w(), h());
          },

          .parent{this},
      }}

    , m_mapImage
      {{
          .texLoadFunc = [this]{ return getMiniMapTexture(); },
          .modColor    = [this]{ return m_alphaOn ? (colorf::WHITE + colorf::A_SHF(128)) : colorf::WHITE_A255; },
          .parent{this},
      }}

    , m_canvas
      {{
          .w = [this]{ return w(); },
          .h = [this]{ return h(); },

          .drawFunc = [this](int drawDstX, int drawDstY)
          {
              if(getMiniMapTexture()){
                  drawCanvas(drawDstX, drawDstY);
                  g_sdlDevice->drawRectangle(colorf::RGBA(231, 231, 189, 128), drawDstX, drawDstY, w(), h());
              }
          },

          .parent{this},
      }}

    , m_cornerUpLeft
      {{
          .texLoadFunc = []{ return g_progUseDB->retrieve(0X09000006); },
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
              if(!getMiniMapTexture()){
                  return;
              }

              flipAlpha();

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
              if(!getMiniMapTexture()){
                  return;
              }

              flipExtended();

              if(m_extended){
                  m_buttonExtend.setTexID({0X09000005, 0X09000005, 0X09000005});
              }
              else{
                  m_buttonExtend.setTexID({0X09000004, 0X09000004, 0X09000004});
              }
          },
      }}

    , m_buttonAutoCenter
      {{
          .texIDList
          {
              .off  = 0X09000004,
              .on   = 0X09000004,
              .down = 0X09000004,
          },

          .onTrigger = [this](Widget *, int)
          {
              if(!getMiniMapTexture()){
                  return;
              }

              flipAutoCenter();

              if(m_autoCenter){
                  m_buttonExtend.setTexID({0X09000005, 0X09000005, 0X09000005});
              }
              else{
                  m_buttonExtend.setTexID({0X09000004, 0X09000004, 0X09000004});
              }
          },
      }}

    , m_buttonFlex
      {{
          .dir = DIR_DOWNRIGHT,

          .x = [this]{ return w() - 1; },
          .y = [this]{ return h() - 1; },

          .itemSpace = 1,
          .childList
          {
              {&m_buttonAlpha, false},
              {&m_buttonExtend, false},
              {&m_buttonAutoCenter, false},
          },

          .parent{this},
      }}
{
    m_mapImage.setSize([this]
    {
        if(auto texPtr = getMiniMapTexture()){
            return to_dround(SDLDeviceHelper::getTextureWidth(texPtr) * m_zoomFactor);
        }
        return 0;
    },

    [this]
    {
        if(auto texPtr = getMiniMapTexture()){
            return to_dround(SDLDeviceHelper::getTextureHeight(texPtr) * m_zoomFactor);
        }
        return 0;
    });

    m_mapImage.moveAt(DIR_UPLEFT, [this]
    {
        if(!getMiniMapTexture()){
            return 0;
        }

        if(!m_autoCenter){
            return 0;
        }

        if(m_mapImage.w() <= w()){
            return (w() - m_mapImage.w()) / 2;
        }

        return std::clamp<int>(w() / 2 - std::get<0>(onMapImagePLoc_from_onMapGLoc(m_processRun->getMyHero()->location())), w() - m_mapImage.w(), 0); // never positive
    },

    [this]
    {
        if(!getMiniMapTexture()){
            return 0;
        }

        if(!m_autoCenter){
            return 0;
        }

        if(m_mapImage.h() <= h()){
            return (h() - m_mapImage.h()) / 2;
        }

        return std::clamp<int>(h() / 2 - std::get<1>(onMapImagePLoc_from_onMapGLoc(m_processRun->getMyHero()->location())), h() - m_mapImage.h(), 0); // never positive
    });

    setShow([this] -> bool { return getMiniMapTexture(); });
}

bool MiniMapBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(!valid){
        m_buttonAlpha .setOff();
        m_buttonExtend.setOff();
        return false;
    }

    bool took = false;
    took |= m_buttonAlpha     .processEvent(event, valid && !took, m.create(m_buttonAlpha     .roi(this)));
    took |= m_buttonExtend    .processEvent(event, valid && !took, m.create(m_buttonExtend    .roi(this)));
    took |= m_buttonAutoCenter.processEvent(event, valid && !took, m.create(m_buttonAutoCenter.roi(this)));

    if(took){
        return true;
    }

    switch(event.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                if(event.button.button == SDL_BUTTON_RIGHT){
                    if(m.in(event.button.x, event.button.y)){
                        const auto onCanvasPX = event.button.x - m.x + m.ro->x;
                        const auto onCanvasPY = event.button.y - m.y + m.ro->y;
                        const auto [onMapPX, onMapPY] = onMapGLoc_from_onCanvasPLoc({onCanvasPX, onCanvasPY});
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

void MiniMapBoard::flipAlpha()
{
    m_alphaOn = !m_alphaOn;
    m_buttonAlpha     .setOff();
    m_buttonExtend    .setOff();
    m_buttonAutoCenter.setOff();
}

void MiniMapBoard::flipExtended()
{
    m_extended = !m_extended;
    m_buttonAlpha     .setOff();
    m_buttonExtend    .setOff();
    m_buttonAutoCenter.setOff();
}

void MiniMapBoard::flipAutoCenter()
{
    m_autoCenter = !m_autoCenter;
    m_buttonAlpha     .setOff();
    m_buttonExtend    .setOff();
    m_buttonAutoCenter.setOff();
}

void MiniMapBoard::drawCanvas(int drawDstX, int drawDstY)
{
    fflassert(getMiniMapTexture());
    for(const auto &p: m_processRun->getCOList()){
        const auto [color, r] = [this](uint64_t uid) -> std::tuple<uint32_t, int>
        {
            switch(uidf::getUIDType(uid)){
                case UID_PLY:
                    {
                        if(uid == m_processRun->getMyHeroUID()){
                            return {colorf::RGBA(255, 0, 255, 255), 3};
                        }
                        else{
                            return {colorf::RGBA(200, 0, 200, 255), 2};
                        }
                    }
                case UID_NPC:
                    {
                        return {colorf::BLUE_A255, 2};
                    }
                case UID_MON:
                    {
                        return {colorf::RED_A255, 1};
                    }
                default:
                    {
                        return {0, 0};
                    }
            }
        }(p.first);

        if(colorf::A(color)){
            if(const auto [onCanvasPX, onCanvasPY] = onCanvasPLoc_from_onMapGLoc(p.second->location()); m_canvas.roi().in(onCanvasPX, onCanvasPY)){
                g_sdlDevice->fillCircle(color, drawDstX + onCanvasPX, drawDstY + onCanvasPY, r);
            }
        }
    }

    if(Widget::ROIMap m{.x{drawDstX}, .y{drawDstY}, .ro{m_canvas.roi()}}; m.crop(m_mapImage.roi(&m_canvas))){
        if(const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc(); m.in(mousePX, mousePY)){

            const auto [onMapGX, onMapGY] = onMapGLoc_from_onCanvasPLoc({mousePX - drawDstX, mousePY - drawDstY});
            const auto bgColor = (m_processRun->canMove(true, 0, onMapGX, onMapGY) ? colorf::BLACK : colorf::RED) + colorf::A_SHF(200);

            const TextBoard locBoard
            {{
                .textFunc = str_printf("[%d,%d]", onMapGX, onMapGY),
                .font
                {
                    .id = 1,
                    .size = 12,
                    .color = colorf::YELLOW_A255,
                },
            }};

            const MarginWrapper textWrapper
            {{
                .wrapped{const_cast<TextBoard *>(&locBoard)},
                .margin
                {
                    .down = 1,
                    .left = 1,
                    .right = 1,
                },

                .bgDrawFunc = [bgColor](const Widget *self, int drawDstX, int drawDstY)
                {
                    g_sdlDevice->fillRectangle(bgColor, drawDstX, drawDstY, self->w(), self->h());
                },
            }};

            textWrapper.draw({.dir{DIR_DOWNRIGHT}, .x{mousePX}, .y{mousePY}});
        }
    }
}

SDL_Texture *MiniMapBoard::getMiniMapTexture() const
{
    if(const auto miniMapIDOpt = DBCOM_MAPRECORD(m_processRun->mapID()).miniMapID; miniMapIDOpt.has_value()){
        return g_progUseDB->retrieve(miniMapIDOpt.value());
    }
    return nullptr;
}

std::tuple<int, int> MiniMapBoard::onMapGLoc_from_onCanvasPLoc(const std::tuple<int, int> &onCanvasPLoc) const
{
    fflassert(getMiniMapTexture());
    const auto [mapUID, mapW, mapH] = m_processRun->getMap();
    const auto onMapImagePLoc = onMapImagePLoc_from_onCanvasPLoc(onCanvasPLoc);
    return
    {
        std::lround(std::get<0>(onMapImagePLoc) * 1.0 * mapW / m_mapImage.w()),
        std::lround(std::get<1>(onMapImagePLoc) * 1.0 * mapH / m_mapImage.h()),
    };
}

std::tuple<int, int> MiniMapBoard::onCanvasPLoc_from_onMapGLoc(const std::tuple<int, int> &onMapGLoc) const
{
    fflassert(getMiniMapTexture());
    return onCanvasPLoc_from_onMapImagePLoc(onMapImagePLoc_from_onMapGLoc(onMapGLoc));
}

std::tuple<int, int> MiniMapBoard::onMapImagePLoc_from_onMapGLoc(const std::tuple<int, int> &onMapGLoc) const
{
    fflassert(getMiniMapTexture());
    const auto [mapUID, mapW, mapH] = m_processRun->getMap();
    return
    {
        to_dround(std::get<0>(onMapGLoc) * 1.0 * m_mapImage.w() / mapW),
        to_dround(std::get<1>(onMapGLoc) * 1.0 * m_mapImage.h() / mapH),
    };
}

std::tuple<int, int> MiniMapBoard::onMapImagePLoc_from_onCanvasPLoc(const std::tuple<int, int> &onCanvasPLoc) const
{
    fflassert(getMiniMapTexture());
    return
    {
        std::get<0>(onCanvasPLoc) - (m_mapImage.dx() - m_canvas.dx()),
        std::get<1>(onCanvasPLoc) - (m_mapImage.dy() - m_canvas.dy()),
    };
}

std::tuple<int, int> MiniMapBoard::onCanvasPLoc_from_onMapImagePLoc(const std::tuple<int, int> &onImgPLoc) const
{
    fflassert(getMiniMapTexture());
    return
    {
        std::get<0>(onImgPLoc) + (m_mapImage.dx() - m_canvas.dx()),
        std::get<1>(onImgPLoc) + (m_mapImage.dy() - m_canvas.dy()),
    };
}
