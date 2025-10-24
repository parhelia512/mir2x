#include "log.hpp"
#include "controlboard.hpp"

extern Log *g_log;
extern SDLDevice *g_sdlDevice;

ControlBoard::ControlBoard(ProcessRun *argProc, Widget *argParent, bool argAutoDelete)
    : Widget
      {{
          .dir = DIR_DOWNLEFT,

          .y = [this]{ return g_sdlDevice->getRendererHeight() - 1; },
          .w = [this]{ return g_sdlDevice->getRendererWidth ()    ; },
          .h = [this]
          {
              switch(m_mode){
                  case CBM_HIDE:
                      {
                          return CBTitle::UP_HEIGHT;
                      }
                  case CBM_DEF:
                      {
                          return m_middle.h() + CBTitle::UP_HEIGHT;
                      }
                  default:
                      {
                          return m_middleExpand.h() + CBTitle::UP_HEIGHT;
                      }
              }
          },

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_processRun(argProc)
    , m_logBoard
      {{
          .font
          {
              .id = 1,
              .size = 12,
          },

          .lineAlign = LALIGN_JUSTIFY,
      }}

    , m_cmdLine
      {{
          .font
          {
              .id = 1,
              .size = 12,
          },

          .lineAlign = LALIGN_JUSTIFY,
      }}

    , m_left
      {
          DIR_DOWNLEFT,
          0,
          [this](const Widget *){ return h() - 1; },

          argProc,
          this,
          false,
      }

    , m_right
      {
          DIR_DOWNRIGHT,
          [this](const Widget *){ return w() - 1; },
          [this](const Widget *){ return h() - 1; },

          argProc,
          this,
          false,
      }

    , m_title
      {
          DIR_UP,
          [this](const Widget *)
          {
              return (w() - m_left.w() - m_right.w()) / 2;
          },

          [this](const Widget *)
          {
              return CBTitle::UP_HEIGHT;
          },

          argProc,
          this,
          false,
      }

    , m_middle
      {
          DIR_DOWNLEFT,
          [this](const Widget *)
          {
              return m_left.w();
          },

          [this](const Widget *)
          {
              return h() - 1;
          },

          [this](const Widget *)
          {
              return w() - m_left.w() - m_right.w();
          },

          argProc,
          this,
          false,
      }

    , m_middleExpand
      {
          DIR_DOWNLEFT,
          [this](const Widget *)
          {
              return m_left.w();
          },

          [this](const Widget *)
          {
              return h() - 1;
          },

          [this](const Widget *)
          {
              return w() - m_left.w() - m_right.w();
          },

          argProc,
          this,
          false,
      }
{}

void ControlBoard::addParLog(const char *log)
{
    fflassert(str_haschar(log));
    m_logBoard.addParXML(m_logBoard.parCount(), {0, 0, 0, 0}, log);

    m_middle      .m_slider.setValue(1.0f, false);
    m_middleExpand.m_slider.setValue(1.0f, false);
}

void ControlBoard::addLog(int logType, const char *log)
{
    if(!log){
        throw fflerror("null log string");
    }

    switch(logType){
        case CBLOG_ERR:
            {
                g_log->addLog(LOGTYPE_WARNING, "%s", log);
                break;
            }
        default:
            {
                g_log->addLog(LOGTYPE_INFO, "%s", log);
                break;
            }
    }

    tinyxml2::XMLDocument xmlDoc(true, tinyxml2::PEDANTIC_WHITESPACE);
    const char *xmlString = [logType]() -> const char *
    {
        // use hex to give alpha
        // color::String2Color has no alpha component

        switch(logType){
            case CBLOG_SYS: return "<par bgcolor = \"rgb(0x00, 0x80, 0x00)\"></par>";
            case CBLOG_DBG: return "<par bgcolor = \"rgb(0x00, 0x00, 0xff)\"></par>";
            case CBLOG_ERR: return "<par bgcolor = \"rgb(0xff, 0x00, 0x00)\"></par>";
            case CBLOG_DEF:
            default       : return "<par></par>";
        }
    }();

    if(xmlDoc.Parse(xmlString) != tinyxml2::XML_SUCCESS){
        throw fflerror("parse xml template failed: %s", xmlString);
    }

    // to support <, >, / in xml string
    // don't directly pass the raw string to addParXML
    xmlDoc.RootElement()->SetText(log);

    tinyxml2::XMLPrinter printer;
    xmlDoc.Print(&printer);
    m_logBoard.addParXML(m_logBoard.parCount(), {0, 0, 0, 0}, printer.CStr());

    m_middle      .m_slider.setValue(1.0f, false);
    m_middleExpand.m_slider.setValue(1.0f, false);
}

TritexButton *ControlBoard::getButton(const std::string_view &buttonName)
{
    if     (buttonName == "Inventory"    ){ return &m_right.m_buttonInventory    ; }
    else if(buttonName == "HeroState"    ){ return &m_right.m_buttonHeroState    ; }
    else if(buttonName == "HeroMagic"    ){ return &m_right.m_buttonHeroMagic    ; }
    else if(buttonName == "Guild"        ){ return &m_right.m_buttonGuild        ; }
    else if(buttonName == "Team"         ){ return &m_right.m_buttonTeam         ; }
    else if(buttonName == "Quest"        ){ return &m_right.m_buttonQuest        ; }
    else if(buttonName == "Horse"        ){ return &m_right.m_buttonHorse        ; }
    else if(buttonName == "RuntimeConfig"){ return &m_right.m_buttonRuntimeConfig; }
    else if(buttonName == "FriendChat"   ){ return &m_right.m_buttonFriendChat   ; }
    else                                  { return nullptr                       ; }
}

void ControlBoard::onClickSwitchModeButton(int)
{}
