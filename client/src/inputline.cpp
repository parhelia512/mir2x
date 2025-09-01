#include <cmath>
#include <utf8.h>
#include "mathf.hpp"
#include "colorf.hpp"
#include "imeboard.hpp"
#include "inputline.hpp"
#include "sdldevice.hpp"
#include "labelboard.hpp"
#include "clientargparser.hpp"

extern IMEBoard *g_imeBoard;
extern SDLDevice *g_sdlDevice;
extern ClientArgParser *g_clientArgParser;

InputLine::InputLine(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        Widget::VarSize argW,
        Widget::VarSize argH,

        bool argIMEEnabled,

        uint8_t  argFont,
        uint8_t  argFontSize,
        uint8_t  argFontStyle,
        uint32_t argFontColor,

        int      argCursorWidth,
        uint32_t argCursorColor,

        std::function<void()>            argOnTab,
        std::function<void()>            argOnCR,
        std::function<void(std::string)> argOnChange,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),

          std::move(argW),
          std::move(argH),

          {},

          argParent,
          argAutoDelete,
      }

    , m_imeEnabled(argIMEEnabled)
    , m_tpset
      {
          0,
          LALIGN_LEFT,
          false,
          argFont,
          argFontSize,
          argFontStyle,
          argFontColor,
      }
    , m_cursorWidth(argCursorWidth)
    , m_cursorColor(argCursorColor)

    , m_onTab   (std::move(argOnTab))
    , m_onCR    (std::move(argOnCR))
    , m_onChange(std::move(argOnChange))
{}

bool InputLine::processEventDefault(const SDL_Event &event, bool valid, int startDstX, int startDstY, const Widget::ROIOpt &roi)
{
    const auto roiOpt = cropDrawROI(startDstX, startDstY, roi);
    if(!roiOpt.has_value()){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                if(!focus()){
                    return false;
                }

                switch(event.key.keysym.sym){
                    case SDLK_TAB:
                        {
                            if(m_onTab){
                                m_onTab();
                            }
                            return true;
                        }
                    case SDLK_RETURN:
                        {
                            if(m_onCR){
                                m_onCR();
                            }
                            return true;
                        }
                    case SDLK_LEFT:
                        {
                            m_cursor = std::max<int>(0, m_cursor - 1);
                            m_cursorBlink = 0.0;
                            return true;
                        }
                    case SDLK_RIGHT:
                        {
                            if(m_tpset.empty()){
                                m_cursor = 0;
                            }
                            else{
                                m_cursor = std::min<int>(m_tpset.lineTokenCount(0), m_cursor + 1);
                            }
                            m_cursorBlink = 0.0;
                            return true;
                        }
                    case SDLK_BACKSPACE:
                        {
                            if(m_cursor > 0){
                                m_tpset.deleteToken(m_cursor - 1, 0, 1);
                                m_cursor--;

                                if(m_onChange){
                                    m_onChange(m_tpset.getRawString());
                                }
                            }
                            m_cursorBlink = 0.0;
                            return true;
                        }
                    case SDLK_ESCAPE:
                        {
                            setFocus(false);
                            return true;
                        }
                    default:
                        {
                            const char keyChar = SDLDeviceHelper::getKeyChar(event, true);
                            if(!g_clientArgParser->disableIME && m_imeEnabled && g_imeBoard->active() && (keyChar >= 'a' && keyChar <= 'z')){
                                g_imeBoard->gainFocus("", str_printf("%c", keyChar), this, [this](std::string s)
                                {
                                    m_tpset.insertUTF8String(m_cursor, 0, s.c_str());
                                    m_cursor += utf8::distance(s.begin(), s.end());
                                    if(m_onChange){
                                        m_onChange(m_tpset.getRawString());
                                    }
                                });
                            }
                            else if(keyChar != '\0'){
                                m_tpset.insertUTF8String(m_cursor++, 0, str_printf("%c", keyChar).c_str());
                                if(m_onChange){
                                    m_onChange(m_tpset.getRawString());
                                }
                            }

                            m_cursorBlink = 0.0;
                            return true;
                        }
                }
            }
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            {
                if(!in(event.button.x, event.button.y, startDstX, startDstY, roiOpt.value())){
                    return consumeFocus(false);
                }

                if(event.type == SDL_MOUSEBUTTONDOWN){
                    const int eventX = event.button.x - startDstX;
                    const int eventY = event.button.y - startDstY;

                    const auto [cursorX, cursorY] = m_tpset.locCursor(eventX, eventY);
                    if(cursorY != 0){
                        throw fflerror("cursor locates at wrong line");
                    }

                    m_cursor = cursorX;
                    m_cursorBlink = 0.0;
                }

                return consumeFocus(true);
            }
        default:
            {
                return false;
            }
    }
}

void InputLine::drawEx(int dstX, int dstY, const Widget::ROIOpt &roi) const
{
    const auto roiOpt = cropDrawROI(dstX, dstY, roi);
    if(!roiOpt.has_value()){
        return;
    }

    int dstCropX = dstX;
    int dstCropY = dstY;
    int srcCropX = roiOpt->x;
    int srcCropY = roiOpt->y;
    int srcCropW = roiOpt->w;
    int srcCropH = roiOpt->h;

    const int tpsetX = 0;
    const int tpsetY = 0 + (h() - (m_tpset.empty() ? m_tpset.getDefaultFontHeight() : m_tpset.ph())) / 2;

    const auto needDraw = mathf::cropROI(
            &srcCropX, &srcCropY,
            &srcCropW, &srcCropH,
            &dstCropX, &dstCropY,

            w(),
            h(),

            tpsetX, tpsetY, m_tpset.pw(), m_tpset.ph());

    if(needDraw){
        m_tpset.drawEx(dstCropX, dstCropY, srcCropX - tpsetX, srcCropY - tpsetY, srcCropW, srcCropH);
    }

    if(std::fmod(m_cursorBlink, 1000.0) > 500.0){
        return;
    }

    if(!focus()){
        return;
    }

    int cursorY = startDstY + tpsetY;
    int cursorX = startDstX + tpsetX + [this]()
    {
        if(m_tpset.empty() || m_cursor == 0){
            return 0;
        }

        if(m_cursor == m_tpset.lineTokenCount(0)){
            return m_tpset.pw();
        }

        const auto pToken = m_tpset.getToken(m_cursor - 1, 0);
        return pToken->box.state.w1 + pToken->box.state.x + pToken->box.info.w;
    }();

    int cursorW = m_cursorWidth;
    int cursorH = std::max<int>(m_tpset.ph(), h());

    if(mathf::rectangleOverlapRegion(dstX, dstY, srcW, srcH, cursorX, cursorY, cursorW, cursorH)){
        g_sdlDevice->fillRectangle(m_cursorColor, cursorX, cursorY, cursorW, cursorH);
    }

    if(g_clientArgParser->debugDrawInputLine){
        g_sdlDevice->drawRectangle(colorf::BLUE + colorf::A_SHF(255), startDstX, startDstY, w(), h());
    }
}

void InputLine::deleteChar()
{
    m_tpset.deleteToken(m_cursor - 1, 0, 1);
    m_cursor--;
}

void InputLine::insertChar(char ch)
{
    const char rawString[]
    {
        ch, '\0',
    };

    m_tpset.insertUTF8String(m_cursor, 0, rawString);
    m_cursor++;
}

void InputLine::insertUTF8String(const char *utf8Str)
{
    if(str_haschar(utf8Str)){
        m_cursor += m_tpset.insertUTF8String(m_cursor, 0, utf8Str);
    }
}

void InputLine::clear()
{
    m_cursor = 0;
    m_cursorBlink = 0.0;

    if(!m_tpset.empty()){
        m_tpset.clear();

        if(m_onChange){
            m_onChange({});
        }
    }
}

void InputLine::setInput(const char *utf8Str)
{
    m_cursor = 0;
    m_cursorBlink = 0.0;

    m_tpset.clear();
    if(str_haschar(utf8Str)){
        m_cursor = m_tpset.insertUTF8String(m_cursor, 0, utf8Str);
    }

    if(m_onChange){
        m_onChange(m_tpset.getRawString());
    }
}
