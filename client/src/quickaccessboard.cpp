#include <tuple>
#include "totype.hpp"
#include "invpack.hpp"
#include "pngtexdb.hpp"
#include "sysconst.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "quickaccessboard.hpp"

extern PNGTexDB *g_itemDB;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

QuickAccessBoard::Grid::Grid(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        Widget::VarSizeOpt argW,
        Widget::VarSizeOpt argH,

        int argSlot,
        ProcessRun *argProc,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),
          .w = std::move(argW),
          .h = std::move(argH),

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , slot(argSlot)
    , proc(argProc)

    , bg
      {
          DIR_UPLEFT,
          0,
          0,

          [this](const Widget *){ return w(); },
          [this](const Widget *){ return h(); },

          [this](const Widget *self, int drawDstX, int drawDstY)
          {
              if(Widget::ROIMap({.x = drawDstX, .y = drawDstY, .ro = self->roi()}).in(SDLDeviceHelper::getMousePLoc())){
                   g_sdlDevice->fillRectangle(colorf::WHITE + colorf::A_SHF(64), drawDstX, drawDstY, self->w(), self->h());
               }
          },

          this,
          false,
      }

    , item
      {
          DIR_NONE,

          [this](const Widget *){ return w() / 2; },
          [this](const Widget *){ return h() / 2; },

          {},
          {},

          [this](const Widget *) -> SDL_Texture *
          {
              if(const auto &item = proc->getMyHero()->getBelt(slot)){
                  return g_itemDB->retrieve(DBCOM_ITEMRECORD(item.itemID).pkgGfxID | 0X01000000);
              }
              return nullptr;
          },

          false,
          false,
          0,

          colorf::WHITE_A255,
          SDL_BLENDMODE_NONE,

          this,
          false,
      }

    , count
      {
          DIR_UPRIGHT,
          [this](const Widget *){ return w() - 1; },
          0,

          [this](const Widget *) -> std::string
          {
              if(const auto &item = proc->getMyHero()->getBelt(slot); item && (item.count > 1)){
                  return std::to_string(item.count);
              }
              return {};
          },

          1,
          10,
          0,

          colorf::WHITE_A255,
          SDL_BLENDMODE_NONE,

          this,
          false,
      }
{}

QuickAccessBoard::QuickAccessBoard(dir8_t argDir,

        int argX,
        int argY,

        ProcessRun *argProc,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = argDir,

          .x = argX,
          .y = argY,
          .w = {},
          .h = {},

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_processRun(argProc)
    , m_bg
      {
          DIR_UPLEFT,
          0,
          0,

          {},
          {},

          [this](const Widget *)
          {
              return g_progUseDB->retrieve(m_texID);
          },

          false,
          false,
          0,

          colorf::WHITE_A255,
          SDL_BLENDMODE_NONE,

          this,
          false,
      }

    , m_buttonClose
      {
          DIR_UPLEFT,
          263,
          32,
          {0X00000061, 0X00000061, 0X00000062},
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
              setShow(false);
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          this,
      }
{
    for(int slot = 0; slot < 6; ++slot){
        addChild(new Grid
        {
            DIR_UPLEFT,
            std::get<0>(getGridLoc(slot)),
            std::get<1>(getGridLoc(slot)),
            std::get<2>(getGridLoc(slot)),
            std::get<3>(getGridLoc(slot)),

            slot,
            m_processRun,
        },

        true);
    }
}

bool QuickAccessBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.crop(roi())){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(m_buttonClose.processEventParent(event, valid, m)){
        return true;
    }

    switch(event.type){
        case SDL_MOUSEMOTION:
            {
                if((event.motion.state & SDL_BUTTON_LMASK) && (m.in(event.motion.x, event.motion.y) || focus())){
                    const auto remapXDiff = m.x - m.ro->x;
                    const auto remapYDiff = m.y - m.ro->y;

                    const auto [rendererW, rendererH] = g_sdlDevice->getRendererSize();
                    const int maxX = rendererW - w();
                    const int maxY = rendererH - h();

                    const int newX = std::max<int>(0, std::min<int>(maxX, remapXDiff + event.motion.xrel));
                    const int newY = std::max<int>(0, std::min<int>(maxY, remapYDiff + event.motion.yrel));

                    moveBy(newX - remapXDiff, newY - remapYDiff);
                    return consumeFocus(true);
                }
                return consumeFocus(false);
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                switch(event.button.button){
                    case SDL_BUTTON_LEFT:
                        {
                            for(int slot = 0; slot < 6; ++slot){
                                const auto [gridX, gridY, gridW, gridH] = getGridLoc(slot);
                                if(mathf::pointInRectangle(event.button.x, event.button.y, m.x + gridX, m.y + gridY, gridW, gridH)){
                                    if(const auto grabbedItem = m_processRun->getMyHero()->getInvPack().getGrabbedItem()){
                                        const auto &ir = DBCOM_ITEMRECORD(grabbedItem.itemID);
                                        if(ir.beltable()){
                                            m_processRun->requestEquipBelt(grabbedItem.itemID, grabbedItem.seqID, slot);
                                        }
                                        else{
                                            m_processRun->getMyHero()->getInvPack().add(grabbedItem);
                                            m_processRun->getMyHero()->getInvPack().setGrabbedItem({});
                                        }
                                    }
                                    else if(m_processRun->getMyHero()->getBelt(slot)){
                                        m_processRun->requestGrabBelt(slot);
                                    }
                                    break;
                                }
                            }

                            if(m.in(event.button.x, event.button.y)){
                                return consumeFocus(true);
                            }
                            else{
                                return consumeFocus(false);
                            }
                        }
                    case SDL_BUTTON_RIGHT:
                        {
                            for(int slot = 0; slot < 6; ++slot){
                                const auto [gridX, gridY, gridW, gridH] = getGridLoc(slot);
                                if(mathf::pointInRectangle(event.button.x, event.button.y, m.x + gridX, m.y + gridY, gridW, gridH)){
                                    gridConsume(slot);
                                    break;
                                }
                            }

                            if(m.in(event.button.x, event.button.y)){
                                return consumeFocus(true);
                            }
                            else{
                                return consumeFocus(false);
                            }
                        }
                    default:
                        {
                            return consumeFocus(false);
                        }
                }
            }
        case SDL_KEYDOWN:
            {
                if(focus()){
                    if(const auto ch = SDLDeviceHelper::getKeyChar(event, false); ch >= '1' && ch <= '6'){
                        gridConsume(ch - '1');
                    }
                    return consumeFocus(true);
                }
                return consumeFocus(false);
            }
        default:
            {
                return false;
            }
    }
}

void QuickAccessBoard::gridConsume(int slot)
{
    fflassert(slot >= 0, slot);
    fflassert(slot <  6, slot);

    if(const auto &item = m_processRun->getMyHero()->getBelt(slot)){
        InvPack::playItemSoundEffect(item.itemID, true);
        m_processRun->requestConsumeItem(item.itemID, item.seqID, 1);
    }
}
