#include "widget.hpp"
#include "colorf.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "gui/controlboard/controlboard.hpp"
#include "inputstringboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

InputStringBoard::InputStringBoard(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        bool argSecurity,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),

          0, // need reset
          0,

          {},

          argParent,
          argAutoDelete,
      }

    , m_bg
      {
          DIR_UPLEFT,
          0,
          0,

          {},
          {},

          [](const Widget *)
          {
              return g_progUseDB->retrieve(0X07000000);
          },

          false,
          false,
          0,

          colorf::WHITE_A255,
          SDL_BLENDMODE_BLEND,

          this,
          false,
      }

    , m_textInfo
      {
          DIR_NONE,
          [this](){ return w() / 2; },
          120,
          250, // line width

          nullptr,
          0,

          {},

          false,
          false,
          false,
          false,

          1,
          12,
          0,

          colorf::WHITE_A255,
          0U,

          LALIGN_JUSTIFY,
          0,
          0,

          0,
          colorf::WHITE_A255,

          nullptr,
          nullptr,
          nullptr,

          this,
          false,
      }

    , m_inputBg
      {
          DIR_UPLEFT,
          22,
          225,

          315,
          23,

          [](const Widget *self, int drawDstX, int drawDstY)
          {
              g_sdlDevice->fillRectangle(colorf::WHITE + colorf::A_SHF(32), drawDstX, drawDstY, self->w(), self->h());
          },

          this,
          false,
      }

    , m_input
      {
          DIR_UPLEFT,
          [this]{ return m_inputBg.dx(); },
          [this]{ return m_inputBg.dy(); },

          [this]{ return m_inputBg.w(); },
          [this]{ return m_inputBg.h(); },

          argSecurity,

          1,
          14,
          0,

          colorf::WHITE_A255,

          2,
          colorf::WHITE_A255,

          nullptr,
          [this]()
          {
              inputLineDone();
              setShow(false);
          },

          this,
          false,
      }

    , m_yesButton
      {
          DIR_UPLEFT,
          66,
          190,

          {0X07000001, 0X07000002, 0X07000003},
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
              inputLineDone();
              setShow(false);
          },

          0,
          0,
          0,
          0,

          true,
          false,
          false,

          this,
          false,
      }

    , m_nopButton
      {
          DIR_UPLEFT,
          212,
          190,

          {0X07000004, 0X07000005, 0X07000006},
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
              m_input.clear();
          },

          0,
          0,
          0,
          0,

          true,
          false,
          false,

          this,
          false,
      }
{
    setShow(false);
    setSize([this]{ return m_bg.w(); },
            [this]{ return m_bg.h(); });

    m_inputBg.setShow([this]
    {
        return m_input.focus();
    });
}

void InputStringBoard::inputLineDone()
{
    const std::string fullInput = m_input.getPasswordString();
    const auto inputPos = fullInput.find_first_not_of(" \n\r\t");
    const std::string realInput = (inputPos == std::string::npos) ? "" : fullInput.substr(inputPos);

    m_input.clear();
    m_input.setFocus(false);

    if(m_onDone){
        m_onDone(to_u8cstr(realInput));
    }
}

void InputStringBoard::waitInput(std::u8string layoutString, std::function<void(std::u8string)> onDone)
{
    m_textInfo.loadXML(to_cstr(layoutString), 0);
    m_onDone = std::move(onDone);

    clear();
    setShow(true);
    setFocus(true);
}
