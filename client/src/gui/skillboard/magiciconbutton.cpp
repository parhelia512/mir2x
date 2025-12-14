#include "magiciconbutton.hpp"
#include "skillboard.hpp"
#include "processrun.hpp"

MagicIconButton::MagicIconButton(
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

void MagicIconButton::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }

    if(const auto levelOpt = m_config->getMagicLevel(magicID()); levelOpt.has_value()){
        Widget::drawDefault(m);

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

bool MagicIconButton::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
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
