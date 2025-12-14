#include "sdldevice.hpp"
#include "skillpage.hpp"
#include "skillboard.hpp"
#include "magiciconbutton.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

SkillPage::SkillPage(uint32_t pageImage, SkillBoardConfig *configPtr, ProcessRun *proc, Widget *argParent, bool argAutoDelete)
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

void SkillPage::addIcon(uint32_t argMagicID)
{
    for(auto buttonCPtr: m_magicIconButtonList){
        if(buttonCPtr->magicID() == argMagicID){
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

void SkillPage::drawDefault(Widget::ROIMap m) const
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
    Widget::drawDefault(m);
}
