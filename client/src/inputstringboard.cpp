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
        Widget::VarInt argX,
        Widget::VarInt argY,

        bool argSecurity,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),
          0,
          0,
          {},

          argParent,
          argAutoDelete,
      }

    , m_input
      {
          DIR_UPLEFT,
          22,
          225,
          315,
          23,
          argSecurity,

          1,
          14,

          0,
          colorf::WHITE + colorf::A_SHF(255),

          2,
          colorf::WHITE + colorf::A_SHF(255),

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
    if(auto texPtr = g_progUseDB->retrieve(0X07000000); !texPtr){
        throw fflerror("no valid purchase count board frame texture");
    }

    setSize([this](const Widget *){ return SDLDeviceHelper::getTextureWidth (g_progUseDB->retrieve(0X07000000)); },
            [this](const Widget *){ return SDLDeviceHelper::getTextureHeight(g_progUseDB->retrieve(0X07000000)); });
}

void InputStringBoard::update(double ms)
{
    m_input.update(ms);
}

void InputStringBoard::drawEx(int dstX, int dstY, const Widget::ROIOpt &roi) const
{
    const auto roiOpt = cropDrawROI(dstX, dstY, roi);
    if(!roiOpt.has_value()){
        return;
    }

    if(auto texPtr = g_progUseDB->retrieve(0X07000000)){
        g_sdlDevice->drawTexture(texPtr, dstX, dstY);
    }

    drawChildEx(&m_input    , dstX, dstY, roiOpt.value());
    drawChildEx(&m_yesButton, dstX, dstY, roiOpt.value());
    drawChildEx(&m_nopButton, dstX, dstY, roiOpt.value());

    const LayoutBoard textInfoBoard
    {
        DIR_UPLEFT,
        0,
        0,
        250,

        to_cstr(m_parXMLString),
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
    };

    drawAsChildEx(&textInfoBoard, DIR_NONE, w() / 2, 120, dstX, dstY, roiOpt.value());

    if(m_input.focus()){
        g_sdlDevice->fillRectangle(colorf::WHITE + colorf::A_SHF(32), dstX + m_input.dx(), dstY + m_input.dy(), m_input.w(), m_input.h());
    }
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
    m_parXMLString = std::move(layoutString);
    m_onDone = std::move(onDone);

    clear();
    setShow(true);
    setFocus(true);
}
