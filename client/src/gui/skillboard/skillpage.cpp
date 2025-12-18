#include "sdldevice.hpp"
#include "skillpage.hpp"
#include "skillboard.hpp"
#include "magiciconbutton.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

SkillPage::SkillPage(SkillPage::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .parent = std::move(args.parent),
      }}

    , m_processRun(fflcheck(args.proc))
    , m_config(fflcheck(args.config))

    , m_bg
      {{
          .texLoadFunc = [texID = args.pageTexID]{ return g_progUseDB->retrieve(texID); },
          .parent{this},
      }}
{
    setSize([this]{ return m_bg.w(); },
            [this]{ return m_bg.h(); });
}

void SkillPage::addIcon(uint32_t argMagicID)
{
    for(auto button: m_magicIconButtonList){
        if(button->magicID() == argMagicID){
            return;
        }
    }

    fflassert(DBCOM_MAGICRECORD(argMagicID));
    m_magicIconButtonList.push_back(new MagicIconButton
    {{
        .magicID = argMagicID,

        .config = m_config,
        .proc   = m_processRun,

        .parent
        {
            .widget = this,
            .autoDelete = true,
        },
    }});
}
