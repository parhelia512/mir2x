#include "fontexdb.hpp"
#include "textboard.hpp"

extern FontexDB *g_fontexDB;
TextBoard::TextBoard(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        std::function<std::string(const Widget *)> argTextFunc,

        uint8_t argFont,
        uint8_t argFontSize,
        uint8_t argFontStyle,

        Widget::VarU32 argColor,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),

          {},
          {},
          {},

          argParent,
          argAutoDelete,
      }

    , m_font(argFont)
    , m_fontSize(argFontSize)
    , m_fontStyle(argFontStyle)
    , m_textFunc(std::move(argTextFunc))

    , m_image
      {
          DIR_UPLEFT,
          0,
          0,

          {}, // image width
          {}, // image height

          [this](const Widget *) -> SDL_Texture *
          {
              return m_textFunc ? g_fontexDB->retrieve(m_font, m_fontSize, m_fontStyle, m_textFunc(this).c_str()) : nullptr;
          },

          false,
          false,
          0,

          std::move(argColor),

          this,
          false,
      }
{
    if(!g_fontexDB->hasFont(argFont)){
        throw fflerror("invalid font: %hhu", argFont);
    }
}
