#include "fflerror.hpp"
#include "sdldevice.hpp"
#include "imeboard.hpp"
#include "processrun.hpp"
#include "guimanager.hpp"
#include "clientargparser.hpp"

extern IMEBoard *g_imeBoard;
extern SDLDevice *g_sdlDevice;
extern ClientArgParser *g_clientArgParser;

GUIManager::GUIManager(ProcessRun *argProc)
    : Widget
      {{
          .w = []{ return g_sdlDevice->getRendererWidth();  },
          .h = []{ return g_sdlDevice->getRendererHeight(); },
      }}

    , m_processRun(argProc)
    , m_NPCChatBoard
      {
          DIR_UPLEFT,
          0,
          0,
          argProc,
      }

    , m_controlBoard
      {
          argProc,
      }

    , m_friendChatBoard
      {
          g_sdlDevice->getRendererWidth()  / 2 - 250,
          g_sdlDevice->getRendererHeight() / 2 - 250,
          argProc,
          this,
      }

    , m_horseBoard
      {
          DIR_UPLEFT,
          g_sdlDevice->getRendererWidth()  / 2 - 128,
          g_sdlDevice->getRendererHeight() / 2 - 161,
          argProc,
          this,
      }

    , m_skillBoard
      {
          g_sdlDevice->getRendererWidth()  / 2 - 180,
          g_sdlDevice->getRendererHeight() / 2 - 224,
          argProc,
          this,
      }

    , m_guildBoard
      {
          g_sdlDevice->getRendererWidth()  / 2 - 297,
          g_sdlDevice->getRendererHeight() / 2 - 222,
          argProc,
          this,
      }

    , m_miniMapBoard
      {{
          .proc = argProc,
      }}

    , m_acutionBoard
      {
          argProc,
          this,
      }

    , m_purchaseBoard
      {
          argProc,
          this,
      }

    , m_teamStateBoard
      {
          g_sdlDevice->getRendererWidth()  / 2 - 129,
          g_sdlDevice->getRendererHeight() / 2 - 122,
          argProc,
          this,
      }

    , m_inventoryBoard
      {{
          .x = g_sdlDevice->getRendererWidth()  / 2 - 141,
          .y = g_sdlDevice->getRendererHeight() / 2 - 233,

          .runProc = argProc,
          .parent{this},
      }}

    , m_questStateBoard
      {
          DIR_UPLEFT,
          g_sdlDevice->getRendererWidth()  / 2 - 145,
          g_sdlDevice->getRendererHeight() / 2 - 223,
          argProc,
          this,
      }

    , m_quickAccessBoard
      {
          DIR_UPLEFT,
          0,
          g_sdlDevice->getRendererHeight() - m_controlBoard.h() - 48,
          argProc,
          this,
      }

    , m_playerStateBoard
      {
          g_sdlDevice->getRendererWidth()  / 2 - 164,
          g_sdlDevice->getRendererHeight() / 2 - 233,
          argProc,
          this,
      }

    , m_inputStringBoard
      {
          DIR_UPLEFT,
          g_sdlDevice->getRendererWidth()  / 2 - 179,
          g_sdlDevice->getRendererHeight() / 2 - 134,
          false,
          this,
      }

    , m_runtimeConfigBoard
      {
          g_sdlDevice->getRendererWidth()  / 2 - 255,
          g_sdlDevice->getRendererHeight() / 2 - 234,

          600,
          480,

          argProc,
          this,
      }

    , m_securedItemListBoard
      {
          0,
          0,
          m_processRun,
          this,
      }
{
    fflassert(m_processRun);
    if(!g_clientArgParser->disableIME){
        g_imeBoard->dropFocus();
    }
}

void GUIManager::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }

    m_miniMapBoard.drawRoot({});
    m_NPCChatBoard.drawRoot({});
    m_controlBoard.drawRoot({});

    if(m_purchaseBoard.show()){
        drawChild(&m_purchaseBoard, m);
    }

    Widget::drawDefault(m);

    if(!g_clientArgParser->disableIME){
        g_imeBoard->drawRoot({});
    }

    ImageBoard img
    {{
        .texLoadFunc = [](const Widget *)
        {
            extern PNGTexDB *g_progUseDB;
            return g_progUseDB->retrieve(0X0100001C);
        },
    }};

    GfxResizeBoard resize
    {{
        .getter = &img,
        .vr
        {
            .x = 10,
            .y = 10,
            .w = 50,
            .h = 50,
        },

        .resize
        {
            100,
            100,
        },
    }};

    img   .drawRoot({.x = 0          });
    resize.drawRoot({.x = 0 + img.w()});

    g_sdlDevice->drawRectangle(colorf::RED_A255, 10, 10, 50, 50);
}

void GUIManager::update(double fUpdateTime)
{
    Widget::update(fUpdateTime);
    m_controlBoard.update(fUpdateTime);
    m_NPCChatBoard.update(fUpdateTime);
    if(!g_clientArgParser->disableIME){
        g_imeBoard->update(fUpdateTime);
    }
}

bool GUIManager::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    switch(event.type){
        case SDL_WINDOWEVENT:
            {
                switch(event.window.event){
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        {
                            afterResize();
                            return true;
                        }
                    default:
                        {
                            break;
                        }
                }
                break;
            }
        default:
            {
                break;
            }
    }

    bool tookEvent = false;
    if(!g_clientArgParser->disableIME){
        tookEvent |= g_imeBoard->processEventRoot(event, valid && !tookEvent, {});
    }

    tookEvent |=        Widget::processEventDefault(event, valid && !tookEvent, m );
    tookEvent |= m_controlBoard.processEventRoot   (event, valid && !tookEvent, {});
    tookEvent |= m_NPCChatBoard.processEventRoot   (event, valid && !tookEvent, {});
    tookEvent |= m_miniMapBoard.processEventRoot   (event, valid && !tookEvent, {});

    return tookEvent;
}

Widget *GUIManager::getWidget(const std::string_view &name)
{
    if(name == "InventoryBoard"){
        return &m_inventoryBoard;
    }

    else if(name == "QuickAccessBoard"){
        return &m_quickAccessBoard;
    }

    else if(name == "NPCChatBoard"){
        return &m_NPCChatBoard;
    }

    else if(name == "ControlBoard"){
        return &m_controlBoard;
    }

    else if(name == "FriendChatBoard"){
        return &m_friendChatBoard;
    }

    else if(name == "HorseBoard"){
        return &m_horseBoard;
    }

    else if(name == "SkillBoard"){
        return &m_skillBoard;
    }

    else if(name == "GuildBoard"){
        return &m_guildBoard;
    }

    else if(name == "MiniMapBoard"){
        return &m_miniMapBoard;
    }

    else if(name == "TeamStateBoard"){
        return &m_teamStateBoard;
    }

    else if(name == "PlayerStateBoard"){
        return &m_playerStateBoard;
    }

    else if(name == "QuestStateBoard"){
        return &m_questStateBoard;
    }

    else if(name == "PurchaseBoard"){
        return &m_purchaseBoard;
    }

    else if(name == "InputStringBoard"){
        return &m_inputStringBoard;
    }

    else if(name == "RuntimeConfigBoard"){
        return &m_runtimeConfigBoard;
    }

    else if(name == "SecuredItemListBoard"){
        return &m_securedItemListBoard;
    }

    else{
        throw fflvalue(name);
    }
}

void GUIManager::afterResizeDefault()
{
    m_controlBoard.afterResize();
    m_runtimeConfigBoard.updateWindowSizeLabel(w(), h(), true);

    const auto fnSetWidgetPLoc = [this](Widget *widgetPtr)
    {
        const auto moveDX = std::max<int>(widgetPtr->dx() - (w() - widgetPtr->w()), 0);
        const auto moveDY = std::max<int>(widgetPtr->dy() - (h() - widgetPtr->h()), 0);

        // move upper-left
        //
        widgetPtr->moveBy(-moveDX, -moveDY);
    };

    if(!g_clientArgParser->disableIME){
        fnSetWidgetPLoc(g_imeBoard);
    }

    fnSetWidgetPLoc(&m_horseBoard);
    fnSetWidgetPLoc(&m_skillBoard);
    fnSetWidgetPLoc(&m_guildBoard);
    fnSetWidgetPLoc(&m_inventoryBoard);
    fnSetWidgetPLoc(&m_quickAccessBoard);
    fnSetWidgetPLoc(&m_playerStateBoard);
    fnSetWidgetPLoc(&m_inputStringBoard);
    fnSetWidgetPLoc(&m_friendChatBoard);
}
