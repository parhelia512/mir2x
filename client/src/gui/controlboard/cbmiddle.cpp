#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <functional>

#include "log.hpp"
#include "colorf.hpp"
#include "totype.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "imageboard.hpp"
#include "processrun.hpp"
#include "controlboard.hpp"
#include "clientmonster.hpp"
#include "teamstateboard.hpp"

extern Log *g_log;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

CBMiddle::CBMiddle(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        Widget::VarSize argW,

        ProcessRun *argProc,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),

          std::move(argW),
          0, // reset

          argParent,
          argAutoDelete,
      }

    , m_processRun(argProc)
    , m_bg
      {
          DIR_UPLEFT,
          0,
          0,

          [this](const Widget *){ return w(); },
          [this](const Widget *){ return h(); },

          [this](const Widget *self, int drawDstX, int drawDstY)
          {
              g_sdlDevice->fillRectangle(colorf::A_SHF(0XFF), drawDstX, drawDstY, self->w(), self->h());
          },

          this,
          false,
      }

    , m_bgImgFull
      {
          DIR_UPLEFT,
          0,
          0,

          {},
          {},

          [](const Widget *)
          {
              return g_progUseDB->retrieve(0X00000013);
          },
      }

    , m_bgImg
      {
          DIR_UPLEFT,
          0,
          0,

          [this](const Widget *){ return             w(); },
          [this](const Widget *){ return m_bgImgFull.h(); },

          &m_bgImgFull,

          50,
          0,
          287,
          [this](const Widget *){ return m_bgImgFull.h(); },

          this,
          false,
      }

    , m_switchMode
      {
          DIR_UPLEFT,
          [this](const Widget *){ return w() - 17; },
          3,

          {SYS_U32NIL, 0X00000028, 0X00000029},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int clickCount)
          {
              if(auto parptr = hasParent<ControlBoard>()){
                  parptr->onClick(clickCount);
              }
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

    // , m_cmdLine
    //   {
    //       DIR_UPLEFT,
    //       7,
    //       105,
    //
    //       [this](const Widget *){ return w() - 6; },
    //       17,
    //
    //       true,
    //
    //       1,
    //       12,
    //       0,
    //       colorf::WHITE + colorf::A_SHF(255),
    //
    //       2,
    //       colorf::WHITE + colorf::A_SHF(255),
    //
    //       nullptr,
    //       [this](){ inputLineDone(); },
    //       nullptr,
    //
    //       this,
    //       false,
    //   }

    , m_logView
      {
          DIR_UPLEFT,
          9,
          11,

          std::addressof(m_logBoard),

          0,
          [this](const Widget *) { return std::max<int>(0, to_dround((m_logBoard.h() - 83) * m_slider.getValue())); },
          [this](const Widget *) { return m_logBoard.w(); },
          83,

          {},

          this,
          false,
      }

    , m_inputBg
      {
          DIR_UPLEFT,
          7,
          104,

          [this](const Widget *){ return w() - 110; },
          17,

          [this](const Widget *self, int drawDstX, int drawDstY)
          {
              if(m_cmdLine.focus()){
                  SDLDeviceHelper::EnableRenderBlendMode enableRenderBlendMode(SDL_BLENDMODE_BLEND);
                  g_sdlDevice->fillRectangle(colorf::GREY + colorf::A_SHF(48), drawDstX, drawDstY, self->w(), self->h());
              }
          },

          this,
          false,
      }

    , m_bgImg
      {
          DIR_UPLEFT,
          0,
          0,

          &m_bgImgFull,

          0,
          0,
          [this](const Widget *){ return w(); },
          [this](const Widget *){ return h(); },

          {},

          this,
          false,
      }

    , m_middle
      {
          DIR_UPLEFT,
          178,
          2, // middle tex height is 131, not 133
          boardW - 178 - 166,
          131,
          {},
          this,
      }

    , m_buttonSwitchMode
      {
          DIR_UPLEFT,
          boardW - 178 - 181,
          3,
          {SYS_U32NIL, 0X00000028, 0X00000029},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *)
          {
              switchExpandMode();
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          &m_middle,
      }

    , m_buttonEmoji
      {
          DIR_UPLEFT,
          boardW - 178 - 260,
          87,
          {SYS_U32NIL, 0X00000023, 0X00000024},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          nullptr,

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          &m_middle,
      }

    , m_buttonMute
      {
          DIR_UPLEFT,
          boardW - 178 - 220,
          87,
          {SYS_U32NIL, 0X00000025, 0X00000026},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          nullptr,

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          &m_middle,
      }

    , m_levelBox
      {
          DIR_NONE,
          m_middle.w() / 2,
          4,
          proc,

          [this](int dy)
          {
              if(!m_expand){
                  return;
              }

              m_stretchH = std::max<int>(m_stretchH - dy, m_stretchHMin);
              m_stretchH = std::min<int>(m_stretchH, g_sdlDevice->getRendererHeight() - 47 - 55);
              setButtonLoc();
          },
          [this]()
          {
              const int winH = g_sdlDevice->getRendererHeight();
              if(!m_expand){
                  switchExpandMode();
                  m_stretchH = winH - 47 - 55;
                  setButtonLoc();
                  return;
              }

              if(m_stretchH != m_stretchHMin){
                  m_stretchH = m_stretchHMin;
              }
              else{
                  m_stretchH = winH - 47 - 55;
              }
              setButtonLoc();
          },
          &m_middle,
      }

    , m_arcAniBoard
      {
          DIR_UPLEFT,
          (boardW - 178 - 166) / 2 - 18,
         -13,
          0X04000000,
          4,
          1,
          true,
          true,
          &m_middle,
      }

    , m_slider
      {
          DIR_UPLEFT,
          boardW - 178 - 176,
          40,
          5,
          60,

          false,
          2,
          nullptr,

          &m_middle,
      }


    , m_logBoard
      {
          DIR_UPLEFT,
          9,
          0, // need reset
          341 + (boardW - 800),

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

          colorf::WHITE + colorf::A_SHF(255),
          0,

          LALIGN_JUSTIFY,
          0,
          0,

          2,
          colorf::WHITE + colorf::A_SHF(255),

          nullptr,
          nullptr,
          nullptr,

          &m_middle,
          false,
      }
{
    if(!proc){
        throw fflerror("invalid ProcessRun provided to CBMiddle()");
    }

    auto fnAssertImage = [](uint32_t img, int w, int h)
    {
        if(auto ptex = g_progUseDB->retrieve(img)){
            int readw = -1;
            int readh = -1;
            if(!SDL_QueryTexture(ptex, 0, 0, &readw, &readh)){
                if(w == readw && h == readh){
                    return;
                }
            }
        }
        throw fflerror("image assertion failed: img = %llu, w = %d, h = %d", to_llu(img), w, h);
    };

    fnAssertImage(0X00000012, 800, 133);
    fnAssertImage(0X00000013, 456, 131);
    fnAssertImage(0X00000022, 127,  41);
    fnAssertImage(0X00000027, 456, 298);

    fflassert(w() == g_sdlDevice->getRendererWidth());
}

std::tuple<int, int> CBMiddle::scheduleStretch(int dstSize, int srcSize)
{
    // use same way for default or expand mode
    // this requires texture 0X00000013 and 0X00000027 are of width 456

    if(dstSize < srcSize){
        return {0, dstSize};
    }

    if(dstSize % srcSize == 0){
        return {dstSize / srcSize, 0};
    }

    const double fillRatio = (1.0 * (dstSize % srcSize)) / srcSize;
    if(fillRatio < 0.5){
        return {dstSize / srcSize - 1, srcSize + (dstSize % srcSize)};
    }
    return {dstSize / srcSize, dstSize % srcSize};
}

void CBMiddle::drawMiddleDefault() const
{
    const int nY0 = dy();
    const int nW0 = w();

    // draw black underlay for the logBoard and actor face
    g_sdlDevice->fillRectangle(colorf::RGBA(0X00, 0X00, 0X00, 0XFF), 178 + 2, nY0 + 14, nW0 - (178 + 2) - (166 + 2), 120);

    m_cmdLine.drawEx(20, 20, 0, 0, m_cmdLine.w(), m_cmdLine.h());
    drawLogBoardDefault();
    drawInputGreyBackground();
    drawFocusFace();

    // draw middle part
    if(auto texPtr = g_progUseDB->retrieve(0X00000013)){
        g_sdlDevice->drawTexture(texPtr,             178, nY0 + 2,         0, 0,  50, 131);
        g_sdlDevice->drawTexture(texPtr, nW0 - 166 - 119, nY0 + 2, 456 - 119, 0, 119, 131);

        const int repeatW = 456 - 50 - 119;
        const int drawW   = nW0 - 50 - 119 - 178 - 166;

        const auto [repeat, stretch] = scheduleStretch(drawW, repeatW);
        for(int i = 0; i < repeat; ++i){
            g_sdlDevice->drawTexture(texPtr, 178 + 50 + i * repeatW, nY0 + 2, 50, 0, repeatW, 131);
        }

        // for the rest area
        // need to stretch or shrink
        if(stretch > 0){
            g_sdlDevice->drawTexture(texPtr, 178 + 50 + repeat * repeatW, nY0 + 2, stretch, 131, 50, 0, repeatW, 131);
        }
    }

    // draw title
    // the title texture is not symmetric, add 1 pixel offset
    // then the levelBox can anchor at the middle by m_middle.w() / 2

    if(auto texPtr = g_progUseDB->retrieve(0X00000022)){
        const auto [titleW, titleH] = SDLDeviceHelper::getTextureSize(texPtr);
        const int titleDstX = 178 + (nW0 - 178 - 166 - titleW) / 2 + 1;
        const int titleDstY = nY0 - 19;
        g_sdlDevice->drawTexture(texPtr, titleDstX, titleDstY);
    }

    m_arcAniBoard.draw();
    m_buttonSwitchMode.draw();
    m_levelBox.draw();
    m_slider.draw();
}

void CBMiddle::drawLogBoardDefault() const
{
    const int dstX = 187;
    const int dstY = logBoardStartY();

    const int srcX = 0;
    const int srcY = std::max<int>(0, std::lround((m_logBoard.h() - 83) * m_slider.getValue()));
    const int srcW = m_logBoard.w();
    const int srcH = 83;

    m_logBoard.drawEx(dstX, dstY, srcX, srcY, srcW, srcH);
}

void CBMiddle::drawLogBoardExpand() const
{
    const int dstX = 187;
    const int dstY = logBoardStartY();

    const int boardFrameH = m_stretchH + 47 + 55 - 70;
    const int srcX = 0;
    const int srcY = std::max<int>(0, std::lround(m_logBoard.h() - boardFrameH) * m_slider.getValue());
    const int srcW = m_logBoard.w();
    const int srcH = boardFrameH;

    m_logBoard.drawEx(dstX, dstY, srcX, srcY, srcW, srcH);
}

void CBMiddle::drawMiddleExpand() const
{
    const int nY0 = dy();
    const int nW0 = w();
    const int nH0 = h();

    // use this position to calculate all points
    // the Y-axis on screen that the big chat-frame starts
    const int startY = nY0 + nH0 - 55 - m_stretchH - 47;

    // draw black underlay for the big log board and input box
    g_sdlDevice->fillRectangle(colorf::RGBA(0X00, 0X00, 0X00, 0XF0), 178 + 2, startY + 2, nW0 - (178 + 2) - (166 + 2), 47 + m_stretchH + 55);

    drawInputGreyBackground();
    m_cmdLine.draw(); // cursor can be over-sized

    if(auto texPtr = g_progUseDB->retrieve(0X00000027)){

        // draw four corners
        g_sdlDevice->drawTexture(texPtr,             178,                   startY,         0,        0,  50, 47);
        g_sdlDevice->drawTexture(texPtr, nW0 - 166 - 119,                   startY, 456 - 119,        0, 119, 47);
        g_sdlDevice->drawTexture(texPtr,             178, startY + 47 + m_stretchH,         0, 298 - 55,  50, 55);
        g_sdlDevice->drawTexture(texPtr, nW0 - 166 - 119, startY + 47 + m_stretchH, 456 - 119, 298 - 55, 119, 55);

        // draw two stretched vertical bars
        const int repeatH = 298 - 47 - 55;
        const auto [repeatHCnt, stretchH] = scheduleStretch(m_stretchH, repeatH);

        for(int i = 0; i < repeatHCnt; ++i){
            g_sdlDevice->drawTexture(texPtr,             178, startY + 47 + i * repeatH,         0, 47,  50, repeatH);
            g_sdlDevice->drawTexture(texPtr, nW0 - 166 - 119, startY + 47 + i * repeatH, 456 - 119, 47, 119, repeatH);
        }

        if(stretchH > 0){
            g_sdlDevice->drawTexture(texPtr,             178, startY + 47 + repeatHCnt * repeatH,  50, stretchH,         0, 47,  50, repeatH);
            g_sdlDevice->drawTexture(texPtr, nW0 - 166 - 119, startY + 47 + repeatHCnt * repeatH, 119, stretchH, 456 - 119, 47, 119, repeatH);
        }

        // draw horizontal top bar and bottom input area
        const int repeatW = 456 - 50 - 119;
        const int drawW   = nW0 - 50 - 119 - 178 - 166;

        const auto [repeatWCnt, stretchW] = scheduleStretch(drawW, repeatW);
        for(int i = 0; i < repeatWCnt; ++i){
            g_sdlDevice->drawTexture(texPtr, 178 + 50 + i * repeatW,                   startY, 50,        0, repeatW, 47);
            g_sdlDevice->drawTexture(texPtr, 178 + 50 + i * repeatW, startY + 47 + m_stretchH, 50, 298 - 55, repeatW, 55);
        }

        if(stretchW > 0){
            g_sdlDevice->drawTexture(texPtr, 178 + 50 + repeatWCnt * repeatW,                   startY, stretchW, 47, 50,        0, repeatW, 47);
            g_sdlDevice->drawTexture(texPtr, 178 + 50 + repeatWCnt * repeatW, startY + 47 + m_stretchH, stretchW, 55, 50, 298 - 55, repeatW, 55);
        }
    }

    if(auto texPtr = g_progUseDB->retrieve(0X00000022)){
        const auto [titleW, titleH] = SDLDeviceHelper::getTextureSize(texPtr);
        const int titleDstX = 178 + (nW0 - 178 - 166 - titleW) / 2 + 1;
        const int titleDstY = startY - 2 - 19;
        g_sdlDevice->drawTexture(texPtr, titleDstX, titleDstY);
    }

    m_arcAniBoard.draw();
    m_buttonSwitchMode.draw();
    m_levelBox.draw();
    m_buttonEmoji.draw();
    m_buttonMute.draw();
    drawLogBoardExpand();
    m_slider.draw();
}

void CBMiddle::drawEx(int, int, int, int, int, int) const
{
    m_left.draw();
    m_right.draw();

    if(m_expand){
        drawMiddleExpand();
    }
    else{
        drawMiddleDefault();
    }
}

bool CBMiddle::processEventDefault(const SDL_Event &event, bool valid, int startDstX, int startDstY)
{
    if(Widget::processEvent(event, valid, startDstX, startDstY)){
        return true;
    }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym){
                    case SDLK_RETURN:
                        {
                            return valid && m_cmdLine.consumeFocus(true);
                        }
                    default:
                        {
                            return false;
                        }
                }
            }
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEMOTION:
        default:
            {
                return false;
            }
    }
}

void CBMiddle::inputLineDone()
{
    const std::string fullInput = m_cmdLine.getRawString();
    const auto inputPos = fullInput.find_first_not_of(" \n\r\t");
    const std::string realInput = (inputPos == std::string::npos) ? "" : fullInput.substr(inputPos);

    m_cmdLine.clear();
    m_cmdLine.setFocus(false);

    if(realInput.empty()){
        return;
    }

    switch(realInput[0]){
        case '!': // broadcast
            {
                break;
            }
        case '@': // user command
            {
                if(m_processRun){
                    m_processRun->userCommand(realInput.c_str() + 1);
                }
                break;
            }
        case '$': // lua command for super user
            {
                if(m_processRun){
                    m_processRun->luaCommand(realInput.c_str() + 1);
                }
                break;
            }
        default: // normal talk
            {
                addLog(0, realInput.c_str());
                break;
            }
    }
}

void CBMiddle::addParLog(const char *log)
{
    fflassert(str_haschar(log));
    m_logBoard.addParXML(m_logBoard.parCount(), {0, 0, 0, 0}, log);
    m_slider.setValue(1.0f, false);
}

void CBMiddle::addLog(int logType, const char *log)
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
    m_slider.setValue(1.0f, false);
}

bool CBMiddle::CheckMyHeroMoved()
{
    return true;
}

void CBMiddle::switchExpandMode()
{
    if(m_expand){
        m_expand = false;
        m_logBoard.setLineWidth(m_logBoard.getLineWidth() - 87);
    }
    else{
        m_expand = true;
        m_stretchH = m_stretchHMin;
        m_logBoard.setLineWidth(m_logBoard.getLineWidth() + 87);
    }
    setButtonLoc();
}

void CBMiddle::setButtonLoc()
{
    // diff of height of texture 0X00000013 and 0X00000027
    // when you draw something on default log board at (X, Y), (0, 0) is left-top
    // if you need to keep the same location on expand log board, draw on(X, Y - modeDiffY)

    const int boardW = w();
    const int modeDiffY = (298 - 131) + (m_stretchH - m_stretchHMin);

    if(m_expand){
        m_buttonSwitchMode.moveTo(boardW - 178 - 181, 3 - modeDiffY);
        m_levelBox.moveTo((boardW - 178 - 166) / 2, 4 - modeDiffY);
        m_arcAniBoard.moveTo((boardW - 178 - 166 - m_arcAniBoard.w()) / 2, -13 - modeDiffY);

        m_buttonEmoji.moveTo(boardW - 178 - 260, 87);
        m_buttonMute .moveTo(boardW - 178 - 220, 87);

        m_slider.moveTo(w() - 178 - 176, 40 - modeDiffY);
        m_slider.setH(60 + modeDiffY);
    }
    else{
        m_buttonSwitchMode.moveTo(boardW - 178 - 181, 3);
        m_levelBox.moveTo((boardW - 178 - 166) / 2, 4);
        m_arcAniBoard.moveTo((boardW - 178 - 166 - m_arcAniBoard.w()) / 2, -13);

        m_slider.moveTo(w() - 178 - 176, 40);
        m_slider.setH(60);
    }
}

int CBMiddle::logBoardStartY() const
{
    if(!m_expand){
        return g_sdlDevice->getRendererHeight() - 120;
    }
    return g_sdlDevice->getRendererHeight() - 55 - m_stretchH - 47 + 12; // 12 is texture top-left to log line distane
}

void CBMiddle::onWindowResize(int winW, int winH)
{
    const auto prevWidth = w();
    setW(winW);

    m_logBoard.setLineWidth(m_logBoard.getLineWidth() + (winW - prevWidth));
    const int maxStretchH = winH - 47 - 55;

    if(m_expand && (m_stretchH > maxStretchH)){
        m_stretchH = maxStretchH;
    }

    m_middle.setW(w() - 178 - 166);
    setButtonLoc();
}

void CBMiddle::drawInputGreyBackground() const
{
    if(!m_cmdLine.focus()){
        return;
    }

    const auto color = colorf::GREY + colorf::A_SHF(48);
    SDLDeviceHelper::EnableRenderBlendMode enableDrawBlendMode(SDL_BLENDMODE_BLEND);

    if(m_expand){

    }
    else{
        g_sdlDevice->fillRectangle(color, m_middle.dx() + 7, m_middle.dy() + 104, m_middle.w() - 110, 17);
    }
}

void CBMiddle::drawRatioBar(int x, int y, double r) const
{
    ImageBoard barImage
    {
        DIR_DOWN,
        0,
        0,

        {},
        {},

        [](const Widget *)
        {
            return g_progUseDB->retrieve(0X000000A0);
        },

        false,
        false,
        0,

        colorf::RGBA(to_u8(255 * r), to_u8(255 * (1 - r)), 0, 255),
    };

    barImage.drawEx(x, y - std::lround(barImage.h() * r), 0, 0, barImage.w(), std::lround(barImage.h() * r));
}

void CBMiddle::drawFocusFace() const
{
    // draw current creature face
    // draw '?' when the face texture is not available

    const auto [faceTexID, hpRatio, buffIDList] = [this]() -> std::tuple<uint32_t, double, const std::optional<SDBuffIDList> &>
    {
        if(const auto coPtr = m_processRun->findUID(m_processRun->getFocusUID(FOCUS_MOUSE))){
            switch(coPtr->type()){
                case UID_PLY:
                    {
                        return
                        {
                            dynamic_cast<Hero *>(coPtr)->faceGfxID(),
                            coPtr->getHealthRatio().at(0),
                            coPtr->getSDBuffIDListOpt(),
                        };
                    }
                case UID_MON:
                    {
                        const auto monFaceTexID = [coPtr]() -> uint32_t
                        {
                            if(const auto lookID = dynamic_cast<ClientMonster*>(coPtr)->lookID(); lookID >= 0){
                                if(const auto texID = to_u32(0X01000000) + (lookID - LID_BEGIN); g_progUseDB->retrieve(texID)){
                                    return texID;
                                }
                            }
                            return 0X010007CF;
                        }();

                        return
                        {
                            monFaceTexID,
                            coPtr->getHealthRatio().at(0),
                            coPtr->getSDBuffIDListOpt(),
                        };
                    }
                default:
                    {
                        break;
                    }
            }
        }

        return
        {
            m_processRun->getMyHero()->faceGfxID(),
            m_processRun->getMyHero()->getHealthRatio().at(0),
            m_processRun->getMyHero()->getSDBuffIDListOpt(),
        };
    }();

    const int nY0 = y();
    const int nW0 = w();

    if(auto faceTexPtr = g_progUseDB->retrieve(faceTexID)){
        const auto [texW, texH] = SDLDeviceHelper::getTextureSize(faceTexPtr);
        g_sdlDevice->drawTexture(faceTexPtr, nW0 - 266, nY0 + 18, 86, 96, 0, 0, texW, texH);

        constexpr int barWidth  = 87;
        constexpr int barHeight =  5;

        if(auto hpBarPtr = g_progUseDB->retrieve(0X00000015)){
            const auto [barTexW, barTexH] = SDLDeviceHelper::getTextureSize(hpBarPtr);
            g_sdlDevice->drawTexture(hpBarPtr, nW0 - 268, nY0 + 15, std::lround(hpRatio * barWidth), barHeight, 0, 0, barTexW, barTexH);
        }
    }

    if(buffIDList.has_value()){
        constexpr int buffIconDrawW = 16;
        constexpr int buffIconDrawH = 16;

        // +---------------+
        // |               |
        // |               |
        // |               |
        // |               |
        // |               |
        // *---------------+
        // ^
        // +--- (buffIconOffStartX, buffIconOffStartY)

        const int buffIconOffStartX = nW0 - 266;
        const int buffIconOffStartY = nY0 +  99;

        for(int drawIconCount = 0; const auto id: buffIDList.value().idList){
            const auto &br = DBCOM_BUFFRECORD(id);
            fflassert(br);

            if(br.icon.gfxID != SYS_U32NIL){
                if(auto iconTexPtr = g_progUseDB->retrieve(br.icon.gfxID)){
                    const int buffIconOffX = buffIconOffStartX + (drawIconCount % 5) * buffIconDrawW;
                    const int buffIconOffY = buffIconOffStartY - (drawIconCount / 5) * buffIconDrawH;

                    const auto [texW, texH] = SDLDeviceHelper::getTextureSize(iconTexPtr);
                    g_sdlDevice->drawTexture(iconTexPtr, buffIconOffX, buffIconOffY, buffIconDrawW, buffIconDrawH, 0, 0, texW, texH);

                    const auto baseColor = [&br]() -> uint32_t
                    {
                        if(br.favor > 0){
                            return colorf::GREEN;
                        }
                        else if(br.favor == 0){
                            return colorf::YELLOW;
                        }
                        else{
                            return colorf::RED;
                        }
                    }();

                    const auto startColor = baseColor | colorf::A_SHF(255);
                    const auto   endColor = baseColor | colorf::A_SHF( 64);

                    const auto edgeGridCount = (buffIconDrawW + buffIconDrawH) * 2 - 4;
                    const auto startLoc = std::lround(edgeGridCount * std::fmod(m_accuTime, 1500.0) / 1500.0);

                    g_sdlDevice->drawBoxFading(startColor, endColor, buffIconOffX, buffIconOffY, buffIconDrawW, buffIconDrawH, startLoc, buffIconDrawW + buffIconDrawH);
                    drawIconCount++;
                }
            }
        }
    }
}
