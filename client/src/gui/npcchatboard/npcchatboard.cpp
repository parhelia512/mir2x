#include <string_view>
#include "uidf.hpp"
#include "totype.hpp"
#include "sysconst.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "npcchatboard.hpp"
#include "clientargparser.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;
extern ClientArgParser *g_clientArgParser;

// with face
// all margins are same
//
// +-----------------------------+
// | +----+ +------------------+ |
// | |    | |                  | |
// | |    | |                  | |
// | |    | |                  | |
// | +----+ +------------------+ |
// +-----------------------------+
//
// without face
// +-----------------------------+
// | +-------------------------+ |
// | |                         | |
// | |                         | |
// | |                         | |
// | +-------------------------+ |
// +-----------------------------+

NPCChatBoard::NPCChatBoard(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        ProcessRun *argProc,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),

          0, // need to reset
          0, // need to reset

          {},
          {},

          argParent,
          argAutoDelete,
      }

    , m_margin(35)
    , m_processRun(argProc)

    , m_face
      {
          DIR_UPLEFT,
          m_margin,
          m_margin,

          {},
          {},

          [this](const Widget *)
          {
              return g_progUseDB->retrieve(getNPCFaceKey());
          },

          false,
          false,
          0,

          colorf::WHITE_A255,
          SDL_BLENDMODE_NONE,

          this,
          false,
      }

    , m_chatBoard
      {
          [this](const Widget *)
          {
              if(g_progUseDB->retrieve(getNPCFaceKey())){
                  return DIR_LEFT;
              }
              else{
                  return DIR_NONE;
              }
          },

          [this](const Widget *)
          {
              if(auto texPtr = g_progUseDB->retrieve(getNPCFaceKey())){
                  return m_margin * 2 + SDLDeviceHelper::getTextureWidth(texPtr);
              }
              else{
                  return w() / 2;
              }
          },

          [this](const Widget *)
          {
              return h() / 2;
          },

          0, // line width, need reset

          nullptr,
          0,

          {0, 0, 0, 0},

          false,
          false,
          false,
          false,

          1,
          12,
          0,

          colorf::WHITE_A255,
          0,

          LALIGN_JUSTIFY,
          0,
          0,

          2,
          colorf::WHITE + colorf::A_SHF(255),

          nullptr,
          nullptr,
          [this](const std::unordered_map<std::string, std::string> &attrList, int event)
          {
              if(event == BEVENT_RELEASE){
                  if(const auto id = LayoutBoard::findAttrValue(attrList, "id", nullptr)){
                      const auto autoClose = [id, closeAttr = LayoutBoard::findAttrValue(attrList, "close", nullptr)]() -> bool
                      {
                          return closeAttr ? to_parsedbool(closeAttr) : false;
                      }();
                      onClickEvent(LayoutBoard::findAttrValue(attrList, "path", m_eventPath.c_str()), id, LayoutBoard::findAttrValue(attrList, "args", nullptr), autoClose);
                  }
              }
          },

          this,
          false,
      }

    , m_buttonClose
      {
          DIR_UPLEFT,
          [this](const Widget *){ return w() - 40; },
          [this](const Widget *){ return h() - 43; },

          {SYS_U32NIL, 0X0000001C, 0X0000001D},
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
          false,
      }
{
    fflassert(m_margin >= 0);
    setShow(false);

    m_face.setShow([this](const Widget *) -> bool
    {
        return g_progUseDB->retrieve(getNPCFaceKey());
    });

    setSize([this](const Widget *)
    {
        if(auto texPtr = g_progUseDB->retrieve(getNPCFaceKey())){
            return m_margin * 3 + SDLDeviceHelper::getTextureWidth(texPtr) + m_chatBoard.w();
        }
        else{
            return m_margin * 2 + m_chatBoard.w();
        }
    },

    [this](const Widget *)
    {
        if(auto texPtr = g_progUseDB->retrieve(getNPCFaceKey())){
            return m_margin * 2 + std::max<int>(SDLDeviceHelper::getTextureWidth(texPtr), m_chatBoard.w());
        }
        else{
            return m_margin * 2 + m_chatBoard.w();
        }
    });
}

void NPCChatBoard::loadXML(uint64_t uid, const char *eventPath, const char *xmlString)
{
    fflassert(uidf::isNPChar(uid), uidf::getUIDString(uid));
    fflassert(str_haschar(eventPath));
    fflassert(str_haschar(xmlString));

    m_npcUID = uid;
    m_eventPath = eventPath;

    m_chatBoard.clear();

    if(auto texPtr = g_progUseDB->retrieve(getNPCFaceKey())){
        m_chatBoard.setLineWidth(w() - m_margin * 3 - SDLDeviceHelper::getTextureWidth(texPtr));
    }
    else{
        m_chatBoard.setLineWidth(w() - m_margin * 2);
    }

    m_chatBoard.loadXML(xmlString);
}

void NPCChatBoard::onClickEvent(const char *path, const char *id, const char *args, bool autoClose)
{
    if(g_clientArgParser->debugClickEvent){
        m_processRun->addCBLog(CBLOG_SYS, u8"clickEvent: path = %s, id = %s, args = %s", to_cstr(path), to_cstr(id), to_cstr(args));
    }

    fflassert(str_haschar(id));
    m_processRun->sendNPCEvent(m_npcUID, path, id, args ? std::make_optional<std::string>(args) : std::nullopt);

    if(autoClose){
        setShow(false);
    }
}
