#include "sdldevice.hpp"
#include "skillpage.hpp"
#include "skillboard.hpp"
#include "magiciconbutton.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

SkillPage::SkillPage(uint32_t argPageTexID, SkillBoardConfig *argConfig, ProcessRun *argProc, Widget *argParent, bool argAutoDelete)
    : Widget
      {{
          .x = SkillBoard::getPageRectange().x,
          .y = SkillBoard::getPageRectange().y,
          .w = SkillBoard::getPageRectange().w,
          .h = SkillBoard::getPageRectange().h,

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_config(fflcheck(argConfig))
    , m_processRun(fflcheck(argProc))
    , m_bg
      {{
          .texLoadFunc = [argPageTexID]{ return g_progUseDB->retrieve(argPageTexID); },
          .parent{this},
      }}
{}

void SkillPage::addIcon(uint32_t argMagicID)
{
    for(auto button: m_magicIconButtonList){
        if(button->magicID() == argMagicID){
            return;
        }
    }

    fflassert(DBCOM_MAGICRECORD(argMagicID));
    const auto &iconGfx = SkillBoard::getMagicIconGfx(argMagicID);

    fflassert(iconGfx);
    m_magicIconButtonList.push_back(new MagicIconButton
    {
        DIR_UPLEFT,
        iconGfx.x * 60 + 12,
        iconGfx.y * 65 + 13,
        argMagicID,
        m_config,
        m_processRun,
        this,
        true,
    });
}
