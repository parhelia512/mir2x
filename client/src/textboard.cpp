#include "fontexdb.hpp"
#include "textboard.hpp"

extern FontexDB *g_fontexDB;
TextBoard::TextBoard(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        VarTextFunc argTextFunc,

        uint8_t argFont,
        uint8_t argFontSize,
        uint8_t argFontStyle,

        Widget::VarU32 argColor,
        Widget::VarBlendMode argBlendMode,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),
          .w = {},
          .h = {},

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_font(argFont)
    , m_fontSize(argFontSize)
    , m_fontStyle(argFontStyle)
    , m_textFunc(std::move(argTextFunc))

    , m_image
      {{
          .texLoadFunc = [this]() -> SDL_Texture *
          {
              if(const auto text = getText(); text.empty()){
                  return nullptr;
              }
              else{
                  return g_fontexDB->retrieve(m_font, m_fontSize, m_fontStyle, text.c_str());
              }
          },

          .modColor = std::move(argColor),
          .blendMode = std::move(argBlendMode),
          .parent{this},
      }}
{
    if(!g_fontexDB->hasFont(argFont)){
        throw fflerror("invalid font: %hhu", argFont);
    }
}

std::string TextBoard::getText() const
{
    switch(m_textFunc.index()){
        case 0 : if(auto &func = std::get<0>(m_textFunc); func){ return func(    ); } else{ return {}; }
        case 1 : if(auto &func = std::get<1>(m_textFunc); func){ return func(this); } else{ return {}; }
        default:                                                                          { return {}; }
    }
}
