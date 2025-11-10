#include "dbcomid.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "skillboard.hpp"
#include "tritexbutton.hpp"
#include "textshadowboard.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

std::optional<char> SkillBoard::SkillBoardConfig::getMagicKey(uint32_t magicID) const
{
    if(auto p = m_learnedMagicList.find(magicID); p != m_learnedMagicList.end()){
        return p->second.key;
    }
    return {};
}

std::optional<int> SkillBoard::SkillBoardConfig::getMagicLevel(uint32_t magicID) const
{
    if(auto p = m_learnedMagicList.find(magicID); p != m_learnedMagicList.end()){
        return p->second.level;
    }
    return {};
}

void SkillBoard::SkillBoardConfig::setMagicLevel(uint32_t magicID, int level)
{
    fflassert(DBCOM_MAGICRECORD(magicID));
    fflassert(SkillBoard::getMagicIconGfx(magicID));

    fflassert(level >= 1);
    fflassert(level <= 3);

    if(auto p = m_learnedMagicList.find(magicID); p != m_learnedMagicList.end()){
        fflassert(level >= p->second.level);
        p->second.level = level;
    }
    else{
        m_learnedMagicList[magicID].level = level;
    }
}

void SkillBoard::SkillBoardConfig::setMagicKey(uint32_t magicID, std::optional<char> key)
{
    fflassert(DBCOM_MAGICRECORD(magicID));
    fflassert(SkillBoard::getMagicIconGfx(magicID));

    fflassert(hasMagicID(magicID));
    fflassert(!SkillBoard::getMagicIconGfx(magicID).passive);
    fflassert(!key.has_value() || (key.value() >= 'a' && key.value() <= 'z') || (key.value() >= '0' && key.value() <= '9'));

    m_learnedMagicList[magicID].key = key;
    if(key.has_value()){
        for(auto &p: m_learnedMagicList){
            if((p.first != magicID) && p.second.key == key){
                p.second.key.reset();
            }
        }
    }
}

SkillBoard::MagicIconButton::MagicIconButton(
        dir8_t argDir,

        int argX,
        int argY,

        uint32_t argMagicID,

        SkillBoardConfig *argConfigPtr,
        ProcessRun       *argProc,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = argDir,

          .x = argX,
          .y = argY,

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_magicID([argMagicID]() -> uint32_t
      {
          fflassert(DBCOM_MAGICRECORD(argMagicID));
          fflassert(SkillBoard::getMagicIconGfx(argMagicID));
          return argMagicID;
      }())

    , m_config(argConfigPtr)
    , m_processRun(argProc)

    , m_icon
      {{
          .texIDList
          {
              .off  = SkillBoard::getMagicIconGfx(argMagicID).magicIcon,
              .on   = SkillBoard::getMagicIconGfx(argMagicID).magicIcon,
              .down = SkillBoard::getMagicIconGfx(argMagicID).magicIcon,
          },

          .onClickDone = false,
          .parent{this},
      }}
{
    // leave some pixels to draw level label
    // since level can change during run, can't get the exact size here
    setW(m_icon.w() + 8);
    setH(m_icon.h() + 8);
}

void SkillBoard::MagicIconButton::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }

    if(const auto levelOpt = m_config->getMagicLevel(magicID()); levelOpt.has_value()){
        Widget::draw(m);

        const LabelBoard magicLevel
        {{
            .label = str_printf(u8"%d", levelOpt.value()).c_str(),
            .font
            {
                .id = 3,
                .size = 12,
                .color = colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
            },
        }};
        magicLevel.draw({.x=(m.x - m.ro->x) + (m_icon.w() - 2), .y=(m.y - m.ro->y) + (m_icon.h() - 1), .ro{m.ro}});

        if(const auto keyOpt = m_config->getMagicKey(magicID()); keyOpt.has_value()){
            const TextShadowBoard magicKey
            {
                DIR_UPLEFT,
                0,
                0,

                2,
                2,

                [keyOpt, this](const Widget *)
                {
                    return str_printf("%c", std::toupper(keyOpt.value()));
                },

                3,
                20,
                0,

                colorf::RGBA(0XFF, 0X80, 0X00, 0XE0),
                colorf::RGBA(0X00, 0X00, 0X00, 0XE0),
            };
            magicKey.draw({.x=(m.x - m.ro->x + 2), .y=(m.y - m.ro->y + 2), .ro=m.ro});
        }
    }
}

SkillBoard::SkillPage::SkillPage(uint32_t pageImage, SkillBoardConfig *configPtr, ProcessRun *proc, Widget *argParent, bool argAutoDelete)
    : Widget
      {{
          .x = SkillBoard::getPageRectange().at(0),
          .y = SkillBoard::getPageRectange().at(1),
          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_config(fflcheck(configPtr))
    , m_processRun(fflcheck(proc))
    , m_pageImage(pageImage)
{
    if(auto texPtr = g_progUseDB->retrieve(m_pageImage)){
        setSize(SDLDeviceHelper::getTextureWidth(texPtr), SDLDeviceHelper::getTextureHeight(texPtr));
    }

    const auto r = SkillBoard::getPageRectange();
    setSize(r[2], r[3]);
}

void SkillBoard::SkillPage::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }

    if(auto texPtr = g_progUseDB->retrieve(m_pageImage)){
        int dstXCrop = m.x;
        int dstYCrop = m.y;
        int srcXCrop = m.ro->x;
        int srcYCrop = m.ro->y;
        int srcWCrop = m.ro->w;
        int srcHCrop = m.ro->h;

        const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
        if(mathf::cropROI(
                    &srcXCrop, &srcYCrop,
                    &srcWCrop, &srcHCrop,
                    &dstXCrop, &dstYCrop,

                    texW,
                    texH)){
            g_sdlDevice->drawTexture(texPtr, dstXCrop, dstYCrop, srcXCrop, srcYCrop, srcWCrop, srcHCrop);
        }
    }
    Widget::draw(m);
}

SkillBoard::SkillBoard(int argX, int argY, ProcessRun *runPtr, Widget *argParent, bool argAutoDelete)
    : Widget
      {{
          .x = argX,
          .y = argY,

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_skillPageList([runPtr, this]() -> std::vector<SkillBoard::SkillPage *>
      {
          std::vector<SkillBoard::SkillPage *> pageList;
          pageList.reserve(8);

          for(int i = 0; i < 8; ++i){
              auto pagePtr = new SkillBoard::SkillPage
              {
                  to_u32(0X05000010 + to_u32(i)),
                  &m_config,
                  runPtr,
                  this,
                  true,
              };

              for(const auto &iconGfx: SkillBoard::getMagicIconGfxList()){
                  if(i == getSkillPageIndex(iconGfx.magicID)){
                      pagePtr->addIcon(iconGfx.magicID);
                  }
              }
              pageList.push_back(pagePtr);
          }
          return pageList;
      }())

    , m_tabButtonList([this]() -> std::vector<TritexButton *>
      {
          std::vector<TritexButton *> tabButtonList;
          tabButtonList.reserve(8);

          const int tabX = 45;
          const int tabY = 10;
          const int tabW = 34;

          // don't know WTF
          // can't compile if use std::vector<TritexButton>
          // looks for complicated parameter list gcc can't forward correctly ???

          for(int i = 0; i < 8; ++i){
              tabButtonList.push_back(new TritexButton
              {{
                  .x = tabX + tabW * i,
                  .y = tabY,

                  .texIDList
                  {
                      .on   = 0X05000020 + to_u32(i),
                      .down = 0X05000030 + to_u32(i),
                  },

                  .onOverIn = [i, this](Widget *)
                  {
                      m_cursorOnTabIndex = i;
                  },

                  .onOverOut = [i, this](Widget *)
                  {
                      if(i != m_cursorOnTabIndex){
                          return;
                      }
                      m_cursorOnTabIndex = -1;
                  },

                  .onTrigger = [i, this](Widget *, int)
                  {
                      if(m_selectedTabIndex == i){
                          return;
                      }

                      m_tabButtonList.at(m_selectedTabIndex)->setOff();

                      m_selectedTabIndex = i;
                      m_slider.setValue(0, false);

                      const auto r = getPageRectange();
                      for(auto pagePtr: m_skillPageList){
                          pagePtr->moveTo(r[0], r[1]);
                      }
                  },

                  .onClickDone = false,
                  .radioMode = true,
                  .alterColor = false,

                  .parent{this},
              }});

              if(i == m_selectedTabIndex){
                  m_tabButtonList.at(i)->setDown();
              }
          }
          return tabButtonList;
      }())

    , m_slider
      {{
          .bar
          {
              .x = 326,
              .y = 74,
              .w = 5,
              .h = 266,
              .v = true,
          },

          .index = 0,
          .onChange = [this](float value)
          {
              const auto r = SkillBoard::getPageRectange();
              auto pagePtr = m_skillPageList.at(m_selectedTabIndex);

              if(r[3] < pagePtr->h()){
                  pagePtr->moveTo(r[0], r[1] - to_d((pagePtr->h() - r[3]) * value));
              }
          },

          .parent{this},
      }}

    , m_closeButton
      {{
          .x = 317,
          .y = 402,

          .texIDList
          {
              .on   = 0X0000001C,
              .down = 0X0000001D,
          },

          .onTrigger = [this](Widget *, int)
          {
              setShow(false);
          },

          .parent{this},
      }}
    , m_processRun(fflcheck(runPtr))
{
    setShow(false);
    if(auto texPtr = g_progUseDB->retrieve(0X05000000)){
        setSize(SDLDeviceHelper::getTextureHeight(texPtr), SDLDeviceHelper::getTextureWidth(texPtr));
    }
    else{
        throw fflerror("no valid inventory frame texture");
    }
}

void SkillBoard::drawDefault(Widget::ROIMap m) const
{
    if(auto texPtr = g_progUseDB->retrieve(0X05000000)){
        g_sdlDevice->drawTexture(texPtr, m.x, m.y);
    }

    drawTabName();
    drawChild(&m_slider, m);
    drawChild(&m_closeButton, m);

    for(auto buttonPtr: m_tabButtonList){
        drawChild(buttonPtr, m);
    }

    const auto r = SkillBoard::getPageRectange();
    auto pagePtr = m_skillPageList.at(m_selectedTabIndex);
    pagePtr->draw({.x=m.x + r[0], .y=m.y + r[1], .ro{r[0] - pagePtr->dx(), r[1] - pagePtr->dy(), r[2], r[3]}});
}

bool SkillBoard::MagicIconButton::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!show()){
        return false;
    }

    if(!m.calibrate(this)){
        return false;
    }

    const auto result = m_icon.processEventParent(event, valid, m);
    if(event.type == SDL_KEYDOWN && cursorOn()){
        if(const auto key = SDLDeviceHelper::getKeyChar(event, false); (key >= '0' && key <= '9') || (key >= 'a' && key <= 'z')){
            if(m_config->hasMagicID(magicID())){
                if(SkillBoard::getMagicIconGfx(magicID()).passive){
                    m_processRun->addCBLog(CBLOG_SYS, u8"无法为被动技能设置快捷键：%s", to_cstr(DBCOM_MAGICRECORD(magicID()).name));
                }
                else{
                    m_config->setMagicKey(magicID(), key);
                    m_processRun->requestSetMagicKey(magicID(), key);
                }
            }
            return consumeFocus(true);
        }
    }
    return result;
}

bool SkillBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(m.calibrate(this)){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    const auto remapXDiff = m.x - m.ro->x;
    const auto remapYDiff = m.y - m.ro->y;

    if(m_closeButton.processEventParent(event, valid, m)){
        consumeFocus(false);
        return true;
    }

    bool tabConsumed = false;
    for(auto buttonPtr: m_tabButtonList){
        tabConsumed |= buttonPtr->processEventParent(event, valid && !tabConsumed, m);
    }

    if(tabConsumed){
        return consumeFocus(true);
    }

    if(m_slider.processEventParent(event, valid, m)){
        return consumeFocus(true);
    }

    bool captureEvent = false;
    if(const auto pageROI = m.create(Widget::makeROI(getPageRectange())); !pageROI.empty()){
        const auto loc = SDLDeviceHelper::getMousePLoc();
        captureEvent = (loc.x >= 0 && loc.y >= 0) && pageROI.in(loc.x - remapXDiff, loc.y - remapYDiff);

        if(m_skillPageList.at(m_selectedTabIndex)->processEventParent(event, captureEvent && valid, m)){
            return consumeFocus(true);
        }
    }

    switch(event.type){
        case SDL_MOUSEMOTION:
            {
                if((event.motion.state & SDL_BUTTON_LMASK) && (m.in(event.motion.x, event.motion.y) || focus())){
                    const auto [rendererW, rendererH] = g_sdlDevice->getRendererSize();
                    const int maxX = rendererW - w();
                    const int maxY = rendererH - h();

                    const int newX = std::max<int>(0, std::min<int>(maxX, remapXDiff + event.motion.xrel));
                    const int newY = std::max<int>(0, std::min<int>(maxY, remapYDiff + event.motion.yrel));

                    moveBy(newX - m.x, newY - m.y);
                    return consumeFocus(true);
                }
                return consumeFocus(false);
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                switch(event.button.button){
                    case SDL_BUTTON_LEFT:
                        {
                            return consumeFocus(m.in(event.button.x, event.button.y));
                        }
                    default:
                        {
                            return consumeFocus(false);
                        }
                }
            }
        case SDL_MOUSEWHEEL:
            {
                auto r = getPageRectange();
                auto pagePtr = m_skillPageList.at(m_selectedTabIndex);

                if(captureEvent && (r[3] < pagePtr->h())){
                    m_slider.addValue(event.wheel.y * -0.1f, false);
                    pagePtr->moveTo(r[0], r[1] - to_d((pagePtr->h() - r[3]) * m_slider.getValue()));
                }
                return consumeFocus(true);
            }
        default:
            {
                return consumeFocus(false);
            }
    }
}

void SkillBoard::drawTabName() const
{
    const LabelBoard tabName
    {{
        .label = to_u8cstr([this]() -> std::u8string
        {
            if(m_cursorOnTabIndex >= 0){
                return str_printf(u8"元素【%s】", to_cstr(magicElemName(cursorOnElem())));
            }

            if(m_selectedTabIndex >= 0){
                for(const auto magicIconPtr: m_skillPageList.at(m_selectedTabIndex)->getMagicIconButtonList()){
                    if(magicIconPtr->cursorOn()){
                        if(const auto &mr = DBCOM_MAGICRECORD(magicIconPtr->magicID())){
                            return str_printf(u8"元素【%s】%s", str_haschar(mr.elem) ? to_cstr(mr.elem) : "无", to_cstr(mr.name));
                        }
                        else{
                            return str_printf(u8"元素【无】");
                        }
                    }
                }
                return str_printf(u8"元素【%s】", to_cstr(magicElemName(selectedElem())));
            }

            // fallback
            // shouldn't reach here
            return str_printf(u8"元素【无】");
        }()),

        .font
        {
            .id = 1,
            .size = 12,
        },
    }};
    // tabName.drawAt(DIR_UPLEFT, x() + 30, y() + 400);
    tabName.draw({.x=30, .y=400});
}
