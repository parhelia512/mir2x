#include <any>
#include <memory>
#include "luaf.hpp"
#include "client.hpp"
#include "imeboard.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "radioselector.hpp"
#include "soundeffectdb.hpp"
#include "processrun.hpp"
#include "inventoryboard.hpp"

extern Client *g_client;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

RuntimeConfigBoard::TextInput::TextInput(
        dir8_t argDir,
        int argX,
        int argY,

        const char8_t *argLabelFirst,
        const char8_t *argLabelSecond,

        int argGapFirst,
        int argGapSecond,

        bool argIMEEnabled,

        int argInputW,
        int argInputH,

        std::function<void()> argOnTab,
        std::function<void()> argOnCR,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = argDir,
          .x = argX,
          .y = argY,
          .w = {},
          .h = {},
          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_labelFirst
      {{
          .label = argLabelFirst,
          .font
          {
              .id = 1,
              .size = 12,
          },
          .parent{this},
      }}

    , m_labelSecond
      {{
          .label = argLabelSecond,
          .font
          {
              .id = 1,
              .size = 12,
          },
          .parent{this},
      }}

    , m_image
      {{
          .texLoadFunc = [](const Widget *){ return g_progUseDB->retrieve(0X00000460); },
      }}

    , m_imageBg
      {{
          .getter = &m_image,
          .vr
          {
              3,
              3,
              m_image.w() - 6,
              2,
          },

          .resize
          {
              argInputW,
              argInputH,
          },

          .parent{this},
      }}

    , m_input
      {
          DIR_UPLEFT,
          0,
          0,

          argInputW,
          argInputH,

          argIMEEnabled,

          1,
          12,
          0,
          colorf::WHITE_A255,

          2,
          colorf::WHITE_A255,

          argOnTab,
          argOnCR,
          nullptr,

          this,
          false,
      }
{
    const int maxH = std::max<int>({m_labelFirst.h(), m_imageBg.h(), m_labelSecond.h()});

    m_labelFirst .moveAt(DIR_LEFT,                                                   0, maxH / 2);
    m_imageBg    .moveAt(DIR_LEFT, m_labelFirst.dx() + m_labelFirst.w() + argGapFirst , maxH / 2);
    m_labelSecond.moveAt(DIR_LEFT, m_imageBg   .dx() + m_imageBg   .w() + argGapSecond, maxH / 2);

    m_input.moveAt(DIR_UPLEFT, m_imageBg.dx() + 3, m_imageBg.dy() + 2);
}

RuntimeConfigBoard::PullMenu::PullMenu(
        dir8_t argDir,
        int argX,
        int argY,

        const char8_t *argLabel,
        int argLabelWidth,

        int argTitleBgWidth,
        int argTitleBgHeight,

        std::initializer_list<std::tuple<Widget *, bool, bool>> argMenuList,
        std::function<void(Widget *)> argOnClickMenu,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = argDir,

          .x = argX,
          .y = argY,
          .w = {},
          .h = {},

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_label
      {{
          .label = argLabel,
          .font
          {
              .id = 1,
              .size = 12,
          },
      }}

    , m_labelCrop
      {{
          .getter = &m_label,
          .roi
          {
              .w = [argLabelWidth]{fflassert(argLabelWidth >= 0); return argLabelWidth; }(),
              .h = m_label.h(),
          },
          .parent{this},
      }}

    , m_menuTitleImage
      {{
          .texLoadFunc = [](const Widget *){ return g_progUseDB->retrieve(0X00000460); },
      }}

    , m_menuTitleBackground
      {{
          .getter = &m_menuTitleImage,
          .vr
          {
              3,
              3,
              m_menuTitleImage.w() - 6,
              2,
          },

          .resize
          {
              .w = fflcheck(argTitleBgWidth , argTitleBgWidth  >= 0),
              .h = fflcheck(argTitleBgHeight, argTitleBgHeight >= 0),
          },

          .parent{this},
      }}

    , m_menuTitle
      {{
          .label = u8"NA",
          .font
          {
              .id = 1,
              .size = 12,
          },
      }}

    , m_menuTitleCrop
      {{
          .getter = &m_menuTitle,
          .roi
          {
              .w = m_menuTitleBackground.w() - 6,
              .h = std::min<int>(m_menuTitleBackground.h() - 4, m_menuTitle.h()),
          },

          .attrs
          {
              .processEvent = [this](Widget *self, const SDL_Event &event, bool valid, Widget::ROIMap m)
              {
                  if(!m.calibrate(self)){
                      return false;
                  }

                  if(!valid){
                      return self->consumeFocus(false);
                  }

                  switch(event.type){
                      case SDL_MOUSEBUTTONUP:
                          {
                              if(m.in(SDLDeviceHelper::getEventPLoc(event).value())){
                                  m_menuList.setShow(true);
                                  return self->consumeFocus(true);
                              }
                              else{
                                  return false;
                              }
                          }
                      default:
                          {
                              return false;
                          }
                  }
              },
          },

          .parent{this},
      }}

    , m_imgOff {{.w = 22, .h = 22, .texLoadFunc = []{ return g_progUseDB->retrieve(0X00000301); }, .rotate = 1}}
    , m_imgOn  {{.w = 22, .h = 22, .texLoadFunc = []{ return g_progUseDB->retrieve(0X00000300); }, .rotate = 1}}
    , m_imgDown{{.w = 22, .h = 22, .texLoadFunc = []{ return g_progUseDB->retrieve(0X00000302); }, .rotate = 1}}

    , m_button
      {{
          .gfxList
          {
              &m_imgOff,
              &m_imgOn,
              &m_imgDown,
          },

          .onTrigger = [this](Widget *, int)
          {
              m_menuList.flipShow();
          },

          .parent{this},
      }}

    , m_menuList
      {
          DIR_UPLEFT,
          0,
          0,

          [this](const Widget *)
          {
              return m_menuTitleCrop.w();
          },

          {5, 5, 5, 5},

          3,
          6,
          7,

          argMenuList,
          std::move(argOnClickMenu),

          this,
          false,
      }
{
    m_menuList.setShow(false);
    const int maxHeight = std::max<int>({m_labelCrop.h(), m_menuTitleBackground.h(), m_button.h()});

    m_labelCrop          .moveAt(DIR_LEFT, 0                                 , maxHeight / 2);
    m_menuTitleBackground.moveAt(DIR_LEFT, m_labelCrop.dx() + m_labelCrop.w(), maxHeight / 2);

    m_menuTitleCrop.moveAt(DIR_LEFT, m_menuTitleBackground.dx() + 3                        , maxHeight / 2);
    m_button       .moveAt(DIR_LEFT, m_menuTitleBackground.dx() + m_menuTitleBackground.w(), maxHeight / 2);

    m_menuList.moveAt(DIR_UPLEFT, m_menuTitleBackground.dx() + 3, m_menuTitleBackground.dy() + m_menuTitleBackground.h() - 2);
}

RuntimeConfigBoard::LabelSliderBar::LabelSliderBar(
        dir8_t argDir,
        int argX,
        int argY,

        const char8_t *argLabel,
        int argLabelWidth,

        int argSliderIndex,
        int argSliderWidth,
        std::function<void(float)> argOnValueChange,

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

    , m_label
      {{
          .label = argLabel,
          .font
          {
              .id = 1,
              .size = 12,
          },
      }}

    , m_labelCrop
      {{
          .getter = &m_label,
          .roi
          {
              .w = fflcheck(argLabelWidth, argLabelWidth >= 0),
              .h = m_label.h(),
          },
          .parent{this},
      }}

    , m_slider
      {
          DIR_UPLEFT,
          0,
          0,
          [argSliderWidth]{fflassert(argSliderWidth >= 0); return argSliderWidth; }(),

          true,
          argSliderIndex,
          std::move(argOnValueChange),

          this,
          false,
      }
{
    setW(m_labelCrop.w() + m_slider.w());
    setH(std::max<int>({m_labelCrop.h(), m_slider.h()}));

    m_labelCrop.moveAt(DIR_LEFT, 0                                 , h() / 2);
    m_slider   .moveAt(DIR_LEFT, m_labelCrop.dx() + m_labelCrop.w(), h() / 2);
}

void RuntimeConfigBoard::PullMenu::drawDefault(Widget::ROIMap m) const
{
    for(const auto p:
    {
        static_cast<const Widget *>(&m_menuTitleBackground),
        static_cast<const Widget *>(&m_labelCrop),
        static_cast<const Widget *>(&m_menuTitleCrop),
        static_cast<const Widget *>(&m_button),
        static_cast<const Widget *>(&m_menuList),
    }){
        if(p->show()){
            drawChild(p, m);
        }
    }
}

bool RuntimeConfigBoard::PullMenu::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(Widget::processEventDefault(event, valid, m)){
        if(!focus()){
            if(!m_menuList.show()){
                consumeFocus(true, &m_button);
            }
            else{
                setFocus(true);
            }
        }
        return true;
    }

    switch(event.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                if(m_menuList.show()){
                    if(!m.create(m_menuList.roi()).in(SDLDeviceHelper::getEventPLoc(event).value())){
                        m_menuList.setShow(false);
                    }
                }
                return false;
            }
        default:
            {
                return false;
            }
    }
}

RuntimeConfigBoard::MenuPage::TabHeader::TabHeader(
        dir8_t argDir,
        int argX,
        int argY,

        const char8_t *argLabel,
        std::function<void(Widget *, int)> argOnClick,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = argDir,

          .x = argX,
          .y = argY,
          .w = {},
          .h = {},

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_label
      {{
          .label = argLabel,
          .font
          {
              .id = 1,
              .size = 14,
          },
      }}

    , m_button
      {{
          .gfxList
          {
              &m_label,
              &m_label,
              &m_label,
          },

          .onTrigger = std::move(argOnClick),

          .offXOnClick = 1,
          .offYOnClick = 1,

          .parent{this},
      }}
{}

RuntimeConfigBoard::MenuPage::MenuPage(
        dir8_t argDir,
        int argX,
        int argY,

        Widget::VarSizeOpt argSeperatorW,
        int argGap,

        std::initializer_list<std::tuple<const char8_t *, Widget *, bool>> argTabList,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = argDir,

          .x = argX,
          .y = argY,
          .w = {},
          .h = {},

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_buttonMask
      {{
          .w = argSeperatorW,
          .h = 0,

          .drawFunc = [this](const Widget *self, int drawDstX, int drawDstY)
          {
              if(m_selectedHeader){
                  g_sdlDevice->fillRectangle(colorf::RGBA(231, 231, 189, 100),
                          drawDstX + m_selectedHeader->dx(),
                          drawDstY + m_selectedHeader->dy(),

                          m_selectedHeader->w(),
                          m_selectedHeader->h());

                  g_sdlDevice->drawLine(colorf::RGBA(231, 231, 189, 100),
                          drawDstX,
                          drawDstY + m_selectedHeader->dy() + m_selectedHeader->h(),

                          drawDstX + self->w(),
                          drawDstY + m_selectedHeader->dy() + m_selectedHeader->h());
              }
          },

          .parent{this},
      }}
{
    TabHeader *lastHeader = nullptr;
    TabHeader *currHeader = nullptr;

    fflassert(argGap >= 0, argGap);
    fflassert(!std::empty(argTabList));

    for(auto &[tabName, tab, autoDelete]: argTabList){
        fflassert(str_haschar(tabName));
        fflassert(tab);

        addChild(tab, autoDelete);
        addChild(currHeader = new TabHeader
        {
            DIR_UPLEFT,
            lastHeader ? (lastHeader->dx() + lastHeader->w() + 10) : 0,
            0,

            tabName,
            [this, tab = tab](Widget *self, int)
            {
                if(m_selectedHeader){
                    std::any_cast<Widget *>(m_selectedHeader->data())->setShow(false);
                }

                tab->setShow(true);
                m_selectedHeader = self->parent();
            }
        }, true);

        currHeader->setData(tab);

        tab->setShow(lastHeader == nullptr);
        tab->moveAt(DIR_UPLEFT, 0, currHeader->dy() + currHeader->h() + argGap);

        lastHeader = currHeader;

        if(!m_selectedHeader){
            m_selectedHeader = currHeader;
        }
    }

    if(!argSeperatorW.has_value()){
        m_buttonMask.setW(w());
    }
    m_buttonMask.setH(h());
}

RuntimeConfigBoard::RuntimeConfigBoard(int argX, int argY, int argW, int argH, ProcessRun *proc, Widget *argParent, bool argAutoDelete)
    : Widget
      {{
           .x = argX,
           .y = argY,
           .w = argW,
           .h = argH,

           .parent
           {
               .widget = argParent,
               .autoDelete = argAutoDelete,
           }
       }}

    , m_frameBoard
      {{
          .w = argW,
          .h = argH,
          .parent{this},
      }}

    , m_leftMenuBackground
      {{
          .x = 30,
          .y = 30,
          .w = 80,
          .h = argH - 30 * 2,

          .drawFunc = [](const Widget *widgetPtr, int drawDstX, int drawDstY)
          {
              g_sdlDevice->fillRectangle(                             colorf::A_SHF(128), drawDstX, drawDstY, widgetPtr->w(), widgetPtr->h(), 10);
              g_sdlDevice->drawRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(100), drawDstX, drawDstY, widgetPtr->w(), widgetPtr->h(), 10);
          },

          .parent{this},
      }}

    , m_leftMenu
      {{
          .dir = DIR_NONE,

          .x = m_leftMenuBackground.dx() + m_leftMenuBackground.w() / 2,
          .y = m_leftMenuBackground.dy() + m_leftMenuBackground.h() / 2,

          .lineWidth = to_dround(m_leftMenuBackground.w() * 0.7),
          .font
          {
              .id = 1,
              .size = 14,
          },

          .onEvent = [this](const std::unordered_map<std::string, std::string> &attrList, int event)
          {
              if(event == BEVENT_RELEASE){
                  if(const auto id = LayoutBoard::findAttrValue(attrList, "id", nullptr)){
                      for(const auto &[label, page]: std::initializer_list<std::tuple<const char8_t *, Widget *>>
                      {
                          {u8"系统", &m_pageSystem},
                          {u8"社交", &m_pageSocial},
                          {u8"游戏", &m_pageGameConfig},
                      }){
                          page->setShow(to_u8sv(id) == label);
                      }
                  }
              }
          },

          .parent{this},
      }}

    , m_pageSystem_resolution
      {
          DIR_UPLEFT,
          0,
          0,

          u8"分辨率",
          40,

          80,
          24,

          {
              {(new LabelBoard{{.label = u8"800×600" , .font{.id = 1, .size = 12}}})->setData(std::make_any<std::pair<int, int>>( 800, 600)), false, true},
              {(new LabelBoard{{.label = u8"960×600" , .font{.id = 1, .size = 12}}})->setData(std::make_any<std::pair<int, int>>( 960, 600)), false, true},
              {(new LabelBoard{{.label = u8"1024×768", .font{.id = 1, .size = 12}}})->setData(std::make_any<std::pair<int, int>>(1024, 768)), false, true},
              {(new LabelBoard{{.label = u8"1280×720", .font{.id = 1, .size = 12}}})->setData(std::make_any<std::pair<int, int>>(1280, 720)), false, true},
              {(new LabelBoard{{.label = u8"1280×768", .font{.id = 1, .size = 12}}})->setData(std::make_any<std::pair<int, int>>(1280, 768)), false, true},
              {(new LabelBoard{{.label = u8"1280×800", .font{.id = 1, .size = 12}}})->setData(std::make_any<std::pair<int, int>>(1280, 800)), false, true},
          },

          [this](Widget *widgetPtr)
          {
              const auto [w, h] = std::any_cast<std::pair<int, int>>(widgetPtr->data());
              g_sdlDevice->setWindowSize(w, h);
              updateWindowSizeLabel(w, h, true);
          },
      }

    , m_pageSystem_musicSlider
      {
          DIR_UPLEFT,
          0,
          0,

          u8"音乐音量",
          60,

          1,
          80,

          [this](float val)
          {
              SDRuntimeConfig_setConfig<RTCFG_BGMVALUE>(m_sdRuntimeConfig, val);
              reportRuntimeConfig(RTCFG_BGMVALUE);
          },
      }

    , m_pageSystem_soundEffectSlider
      {
          DIR_UPLEFT,
          0,
          0,

          u8"声效音量",
          60,

          1,
          80,

          [this](float val)
          {
              SDRuntimeConfig_setConfig<RTCFG_SEFFVALUE>(m_sdRuntimeConfig, val);
              reportRuntimeConfig(RTCFG_SEFFVALUE);
          },
      }

    , m_pageSystem
      {
          DIR_UPLEFT,
          m_leftMenuBackground.dx() + m_leftMenuBackground.w() + 30,
          m_leftMenuBackground.dy() + 10,

          w() - (m_leftMenuBackground.dx() + m_leftMenuBackground.w() + 30) - 50,
          20,

          {
              {
                  u8"系统",
                  new Widget
                  {{
                      .w = {},
                      .h = {},

                      .childList
                      {
                          {&m_pageSystem_resolution, DIR_UPLEFT, 0, 0, false},

                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, nullptr, nullptr, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_FULLSCREEN>(m_sdRuntimeConfig, value); reportRuntimeConfig(RTCFG_FULLSCREEN); }, u8"全屏显示", 1, 12, 0, colorf::WHITE_A255) , DIR_UPLEFT, 0, 40, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, nullptr, nullptr, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_SHOWFPS   >(m_sdRuntimeConfig, value); reportRuntimeConfig(RTCFG_SHOWFPS   ); }, u8"显示FPS" , 1, 12, 0, colorf::WHITE_A255) , DIR_UPLEFT, 0, 65, true},

                          {new CheckLabel
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              false,
                              8,

                              colorf::RGBA(231, 231, 189, 128),
                              16,
                              16,

                              [this](const Widget *)
                              {
                                  return SDRuntimeConfig_getConfig<RTCFG_BGM>(m_sdRuntimeConfig);
                              },

                              [this](Widget *, bool value)
                              {
                                  SDRuntimeConfig_setConfig<RTCFG_BGM>(m_sdRuntimeConfig, value);
                              },

                              [this](Widget *, bool value)
                              {
                                  reportRuntimeConfig(RTCFG_BGM);
                                  m_pageSystem_musicSlider.setActive(value);
                              },

                              u8"背景音乐",
                              1,
                              12,
                              0,
                              colorf::WHITE_A255,
                          },
                          DIR_UPLEFT, 0, 100, true},

                          {&m_pageSystem_musicSlider, DIR_UPLEFT, 0, 125, false},

                          {new CheckLabel
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              false,
                              8,

                              colorf::RGBA(231, 231, 189, 128),
                              16,
                              16,

                              [this](const Widget *)
                              {
                                  return SDRuntimeConfig_getConfig<RTCFG_SEFF>(m_sdRuntimeConfig);
                              },

                              [this](Widget *, bool value)
                              {
                                  SDRuntimeConfig_setConfig<RTCFG_SEFF>(m_sdRuntimeConfig, value);
                              },

                              [this](Widget *, bool value)
                              {
                                  reportRuntimeConfig(RTCFG_SEFF);
                                  m_pageSystem_soundEffectSlider.setActive(value);
                              },

                              u8"动作声效",
                              1,
                              12,
                              0,
                              colorf::WHITE_A255,
                          },
                          DIR_UPLEFT, 0,  160, true},

                          {&m_pageSystem_soundEffectSlider, DIR_UPLEFT, 0, 185, false},
                      },
                  }},
                  true,
              },
          },

          this,
          false,
      }

    , m_pageSocial
      {
          DIR_UPLEFT,
          m_leftMenuBackground.dx() + m_leftMenuBackground.w() + 30,
          m_leftMenuBackground.dy() + 10,

          w() - (m_leftMenuBackground.dx() + m_leftMenuBackground.w() + 30) - 50,
          20,

          {
              {
                  u8"社交",
                  new Widget
                  {{
                      .w = {},
                      .h = {},

                      .childList
                      {
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_允许私聊        >(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_允许私聊        >(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_允许私聊        ); }, u8"允许私聊"        , 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT,   0,   0, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_允许白字聊天    >(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_允许白字聊天    >(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_允许白字聊天    ); }, u8"允许白字聊天"    , 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT,   0,  25, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_允许地图聊天    >(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_允许地图聊天    >(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_允许地图聊天    ); }, u8"允许地图聊天"    , 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT,   0,  50, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_允许行会聊天    >(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_允许行会聊天    >(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_允许行会聊天    ); }, u8"允许行会聊天"    , 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT,   0,  75, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_允许全服聊天    >(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_允许全服聊天    >(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_允许全服聊天    ); }, u8"允许全服聊天"    , 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT,   0, 100, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_允许加入队伍    >(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_允许加入队伍    >(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_允许加入队伍    ); }, u8"允许加入队伍"    , 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 200,   0, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_允许加入行会    >(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_允许加入行会    >(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_允许加入行会    ); }, u8"允许加入行会"    , 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 200,  25, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_允许回生术      >(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_允许回生术      >(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_允许回生术      ); }, u8"允许回生术"      , 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 200,  50, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_允许天地合一    >(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_允许天地合一    >(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_允许天地合一    ); }, u8"允许天地合一"    , 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 200,  75, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_允许交易        >(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_允许交易        >(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_允许交易        ); }, u8"允许交易"        , 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 200, 100, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_允许添加好友    >(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_允许添加好友    >(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_允许添加好友    ); }, u8"允许添加好友"    , 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 200, 125, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_允许行会召唤    >(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_允许行会召唤    >(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_允许行会召唤    ); }, u8"允许行会召唤"    , 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 200, 150, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_允许行会杀人提示>(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_允许行会杀人提示>(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_允许行会杀人提示); }, u8"允许行会杀人提示", 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 200, 175, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_允许拜师        >(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_允许拜师        >(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_允许拜师        ); }, u8"允许拜师"        , 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 200, 200, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_允许好友上线提示>(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_允许好友上线提示>(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_允许好友上线提示); }, u8"允许好友上线提示", 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 200, 225, true},
                      },
                  }},
                  true,
              },

              {
                  u8"好友",
                  new Widget
                  {{
                      .w = {},
                      .h = {},

                      .childList
                      {
                          {new LabelBoard
                          {{
                              .label = u8"当加我为好友时：",
                              .font
                              {
                                  .id = 1,
                                  .size = 12,
                              },

                          }}, DIR_UPLEFT, 0, 0, true},

                          {new RadioSelector
                          {
                              DIR_UPLEFT, // ignored
                              0,
                              0,

                              5,
                              5,

                              {
                                  {(new LabelBoard{{.label = u8"允许任何人加我微好友", .font{.id = 1, .size = 12}}})->setData(to_d(FR_ACCEPT)), true},
                                  {(new LabelBoard{{.label = u8"拒绝任何人加我为好友", .font{.id = 1, .size = 12}}})->setData(to_d(FR_REJECT)), true},
                                  {(new LabelBoard{{.label = u8"好友申请验证"        , .font{.id = 1, .size = 12}}})->setData(to_d(FR_VERIFY)), true},
                              },

                              [this](const Widget *radioSelector)
                              {
                                  const Widget *selectedWidget = nullptr;
                                  const auto val = SDRuntimeConfig_getConfig<RTCFG_好友申请>(m_sdRuntimeConfig);

                                  dynamic_cast<const RadioSelector *>(radioSelector)->foreachRadioWidget([&selectedWidget, val](const Widget *widget)
                                  {
                                      if(std::any_cast<int>(widget->data()) == val){
                                          selectedWidget = widget;
                                          return true;
                                      }
                                      return false;
                                  });

                                  return selectedWidget;
                              },

                              [this](Widget *, Widget *radioWidget)
                              {
                                  SDRuntimeConfig_setConfig<RTCFG_好友申请>(m_sdRuntimeConfig, std::any_cast<int>(radioWidget->data()));
                                  reportRuntimeConfig(RTCFG_好友申请);
                              },

                          }, DIR_UPLEFT, 0, 20, true},
                      },
                  }},
                  true,
              },
          },

          this,
          false,
      }

    , m_pageGameConfig
      {
          DIR_UPLEFT,
          m_leftMenuBackground.dx() + m_leftMenuBackground.w() + 30,
          m_leftMenuBackground.dy() + 10,

          w() - (m_leftMenuBackground.dx() + m_leftMenuBackground.w() + 30) - 50,
          20,

          {
              {
                  u8"常用",
                  new Widget
                  {{
                      .w = {},
                      .h = {},

                      .childList
                      {
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_强制攻击    >(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_强制攻击    >(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_强制攻击    ); }, u8"强制攻击"    , 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 0,   0, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_显示体力变化>(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_显示体力变化>(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_显示体力变化); }, u8"显示体力变化", 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 0,  25, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_满血不显血  >(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_满血不显血  >(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_满血不显血  ); }, u8"满血不显血"  , 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 0,  50, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_显示血条    >(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_显示血条    >(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_显示血条    ); }, u8"显示血条"    , 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 0,  75, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_数字显血    >(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_数字显血    >(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_数字显血    ); }, u8"数字显血"    , 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 0, 100, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_综合数字显示>(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_综合数字显示>(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_综合数字显示); }, u8"综合数字显示", 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 0, 125, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_标记攻击目标>(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_标记攻击目标>(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_标记攻击目标); }, u8"标记攻击目标", 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 0, 150, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_单击解除锁定>(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_单击解除锁定>(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_单击解除锁定); }, u8"单击解除锁定", 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 0, 175, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_显示BUFF图标>(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_显示BUFF图标>(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_显示BUFF图标); }, u8"显示BUFF图标", 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 0, 200, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_显示BUFF计时>(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_显示BUFF计时>(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_显示BUFF计时); }, u8"显示BUFF计时", 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 0, 225, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_显示角色名字>(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_显示角色名字>(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_显示角色名字); }, u8"显示角色名字", 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 0, 250, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_关闭组队血条>(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_关闭组队血条>(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_关闭组队血条); }, u8"关闭组队血条", 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 0, 275, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_队友染色    >(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_队友染色    >(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_队友染色    ); }, u8"队友染色"    , 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 0, 300, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_显示队友位置>(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_显示队友位置>(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_显示队友位置); }, u8"显示队友位置", 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 0, 325, true},
                      },
                  }},
                  true,
              },

              {
                  u8"辅助",
                  new Widget
                  {{
                      .w = {},
                      .h = {},

                      .childList
                      {
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_持续盾      >(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_持续盾      >(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_持续盾      ); }, u8"持续盾"      , 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 0,   0, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_持续移花接木>(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_持续移花接木>(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_持续移花接木); }, u8"持续移花接木", 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 0,  25, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_持续金刚    >(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_持续金刚    >(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_持续金刚    ); }, u8"持续金刚"    , 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 0,  50, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_持续破血    >(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_持续破血    >(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_持续破血    ); }, u8"持续破血"    , 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 0, 100, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_持续铁布衫  >(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_持续铁布衫  >(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_持续铁布衫  ); }, u8"持续铁布衫"  , 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 0, 125, true},
                      },
                  }},
                  true,
              },

              {
                  u8"保护",
                  new Widget
                  {{
                      .w = {},
                      .h = {},

                      .childList
                      {
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_自动喝红>(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_自动喝红>(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_自动喝红); }, u8"自动喝红", 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 0,  0, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_保持满血>(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_保持满血>(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_保持满血); }, u8"保持满血", 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 0, 25, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_自动喝蓝>(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_自动喝蓝>(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_自动喝蓝); }, u8"自动喝蓝", 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 0, 50, true},
                          {new CheckLabel(DIR_UPLEFT, 0, 0, true, 8, colorf::RGBA(231, 231, 189, 128), 16, 16, [this](const Widget *){ return SDRuntimeConfig_getConfig<RTCFG_保持满蓝>(m_sdRuntimeConfig); }, [this](Widget *, bool value){ SDRuntimeConfig_setConfig<RTCFG_保持满蓝>(m_sdRuntimeConfig, value); }, [this](Widget *, bool){ reportRuntimeConfig(RTCFG_保持满蓝); }, u8"保持满蓝", 1, 12, 0, colorf::WHITE_A255), DIR_UPLEFT, 0, 75, true},

                          {new TextInput(DIR_UPLEFT, 0, 0, u8"等待", u8"秒", 3, 3, true, 50, 20), DIR_UPLEFT, 0, 110, true},
                      },
                  }},
                  true,
              },
          },

          this,
          false,
      }

    , m_processRun([proc]()
      {
          fflassert(proc); return proc;
      }())
{
    m_leftMenu.loadXML(
        R"###( <layout>                                                 )###""\n"
        R"###(     <par><event id="系统">系 统</event></par><par></par> )###""\n"
        R"###(     <par><event id="社交">社 交</event></par><par></par> )###""\n"
        R"###(     <par><event id="网络">网 络</event></par><par></par> )###""\n"
        R"###(     <par><event id="游戏">游 戏</event></par><par></par> )###""\n"
        R"###(     <par><event id="帮助">帮 助</event></par><par></par> )###""\n"
        R"###( </layout>                                                )###""\n"
    );

    // 1.0f -> SDL_MIX_MAXVOLUME
    // SDL_mixer initial sound/music volume is SDL_MIX_MAXVOLUME

    m_pageSystem_musicSlider      .getSlider()->setValue(0.0, false);
    m_pageSystem_soundEffectSlider.getSlider()->setValue(0.0, false);

    m_pageSystem.setShow(true);
    m_pageSocial.setShow(false);
    m_pageGameConfig.setShow(false);

    setShow(false);
}

void RuntimeConfigBoard::drawDefault(Widget::ROIMap m) const
{
    for(const auto p:
    {
        static_cast<const Widget *>(&m_frameBoard),
        static_cast<const Widget *>(&m_leftMenuBackground),
        static_cast<const Widget *>(&m_leftMenu),
        static_cast<const Widget *>(&m_pageSystem),
        static_cast<const Widget *>(&m_pageSocial),
        static_cast<const Widget *>(&m_pageGameConfig),
    }){
        if(p->show()){
            drawChild(p, m);
        }
    }
}

bool RuntimeConfigBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    for(auto widgetPtr:
    {
        static_cast<Widget *>(&m_leftMenu),
        static_cast<Widget *>(&m_frameBoard),
        static_cast<Widget *>(&m_pageSystem),
        static_cast<Widget *>(&m_pageSocial),
        static_cast<Widget *>(&m_pageGameConfig),
    }){
        if(widgetPtr->processEventParent(event, valid, m)){
            return true;
        }
    }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                if(event.key.keysym.sym == SDLK_ESCAPE){
                    setShow(false);
                    return consumeFocus(false);
                }
                return consumeFocus(true);
            }
        case SDL_MOUSEMOTION:
            {
                if((event.motion.state & SDL_BUTTON_LMASK) && (m.in(event.motion.x, event.motion.y) || focus())){
                    const auto remapXDiff = m.x - m.ro->x;
                    const auto remapYDiff = m.y - m.ro->y;

                    const auto [rendererW, rendererH] = g_sdlDevice->getRendererSize();
                    const int maxX = rendererW - w();
                    const int maxY = rendererH - h();

                    const int newX = std::max<int>(0, std::min<int>(maxX, remapXDiff + event.motion.xrel));
                    const int newY = std::max<int>(0, std::min<int>(maxY, remapYDiff + event.motion.yrel));

                    moveBy(newX - remapXDiff, newY - remapYDiff);
                    return consumeFocus(true);
                }
		return false;
            }
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            {
                return consumeFocus(m.in(event.button.x, event.button.y));
            }
        default:
            {
                return false;
            }
    }
}

void RuntimeConfigBoard::setConfig(const SDRuntimeConfig &config)
{
    m_sdRuntimeConfig = config;

    m_pageSystem_musicSlider      .setActive(SDRuntimeConfig_getConfig<RTCFG_BGM >(m_sdRuntimeConfig));
    m_pageSystem_soundEffectSlider.setActive(SDRuntimeConfig_getConfig<RTCFG_SEFF>(m_sdRuntimeConfig));

    m_pageSystem_musicSlider      .getSlider()->setValue(SDRuntimeConfig_getConfig<RTCFG_BGMVALUE >(m_sdRuntimeConfig), false);
    m_pageSystem_soundEffectSlider.getSlider()->setValue(SDRuntimeConfig_getConfig<RTCFG_SEFFVALUE>(m_sdRuntimeConfig), false);

    const auto [winW, winH] = SDRuntimeConfig_getConfig<RTCFG_WINDOWSIZE>(m_sdRuntimeConfig);
    g_sdlDevice->setWindowSize(winW, winH);
}

void RuntimeConfigBoard::reportRuntimeConfig(int rtCfg)
{
    fflassert(rtCfg >= RTCFG_BEGIN, rtCfg);
    fflassert(rtCfg <  RTCFG_END  , rtCfg);

    CMSetRuntimeConfig cmSRC;
    std::memset(&cmSRC, 0, sizeof(cmSRC));

    cmSRC.type = rtCfg;
    cmSRC.buf.assign(m_sdRuntimeConfig.getConfig(rtCfg).value_or(std::string()));

    g_client->send({CM_SETRUNTIMECONFIG, cmSRC});
}

void RuntimeConfigBoard::updateWindowSizeLabel(int w, int h, bool saveConfig)
{
    fflassert(w >= 0, w, h);
    fflassert(h >= 0, w, h);

    m_pageSystem_resolution.getMenuTitle()->setText(str_printf(u8"%d×%d", w, h).c_str());
    if(saveConfig){
        SDRuntimeConfig_setConfig<RTCFG_WINDOWSIZE>(m_sdRuntimeConfig, std::make_pair(w, h));
        reportRuntimeConfig(RTCFG_WINDOWSIZE);
    }
}
