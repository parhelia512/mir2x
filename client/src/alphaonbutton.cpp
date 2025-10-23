#include "colorf.hpp"
#include "totype.hpp"
#include "bevent.hpp"
#include "sdldevice.hpp"
#include "alphaonbutton.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

AlphaOnButton::AlphaOnButton(AlphaOnButton::InitArgs args)
    : ButtonBase
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .onOverIn  = std::move(args.onOverIn),
          .onOverOut = std::move(args.onOverOut),

          .onClick = std::move(args.onClick),
          .onTrigger = std::move(args.onTrigger),

          .onClickDone = args.triggerOnDone,
          .parent = args.parent,
      }}

    , m_modColor(std::move(args.modColor))
    , m_downTexID(std::move(args.downTexID))

    , m_onOffX(args.onOffX)
    , m_onOffY(args.onOffY)
    , m_onRadius(args.onRadius)

    , m_on
      {
          DIR_UPLEFT,
          m_onOffX,
          m_onOffY,

          [this](const Widget *){ return w(); },
          [this](const Widget *){ return h(); },

          [this](const Widget *) -> SDL_Texture *
          {
              return (getState() == BEVENT_ON) ? g_sdlDevice->getCover(m_onRadius, 360) : nullptr;
          },

          false,
          false,
          0,

          m_modColor,
          SDL_BLENDMODE_BLEND,

          this,
          false,
      }

    , m_down
      {
          DIR_UPLEFT,
          0,
          0,

          {},
          {},

          [this](const Widget *) -> SDL_Texture *
          {
              return (getState() == BEVENT_DOWN) ? g_progUseDB->retrieve(Widget::evalU32(m_downTexID, this)) : nullptr;
          },

          false,
          false,
          0,

          colorf::WHITE + colorf::A_SHF(0XFF),
          SDL_BLENDMODE_NONE,

          this,
          false,
      }
{
    m_on  .setShow([this](const Widget *){ return getState() == BEVENT_ON  ; });
    m_down.setShow([this](const Widget *){ return getState() == BEVENT_DOWN; });

    setSize([this](const Widget *)
    {
        if(auto texPtr = g_progUseDB->retrieve(Widget::evalU32(m_downTexID, this))){
            return SDLDeviceHelper::getTextureWidth(texPtr);
        }
        return 0;
    },

    [this](const Widget *)
    {
        if(auto texPtr = g_progUseDB->retrieve(Widget::evalU32(m_downTexID, this))){
            return SDLDeviceHelper::getTextureHeight(texPtr);
        }
        return 0;
    });
}
