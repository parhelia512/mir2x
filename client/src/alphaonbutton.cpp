#include "colorf.hpp"
#include "totype.hpp"
#include "bevent.hpp"
#include "sdldevice.hpp"
#include "alphaonbutton.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

AlphaOnButton::AlphaOnButton(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        int argOnOffX,
        int argOnOffY,
        int argOnRadius,

        uint32_t argModColor,
        uint32_t argDownTexID,

        std::function<void(Widget *           )> fnOnOverIn,
        std::function<void(Widget *           )> fnOnOverOut,
        std::function<void(Widget *, bool, int)> fnOnClick,
        std::function<void(Widget *,       int)> fnOnTrigger,

        bool    triggerOnDone,
        Widget *pwidget,
        bool    autoDelete)

    : ButtonBase
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),

          .onOverIn  = std::move(fnOnOverIn),
          .onOverOut = std::move(fnOnOverOut),

          .onClick = std::move(fnOnClick),
          .onTrigger = std::move(fnOnTrigger),

          .onClickDone = triggerOnDone,

          .parent
          {
              .widget = pwidget,
              .autoDelete = autoDelete,
          }
      }}

    , m_modColor(argModColor)
    , m_texID(argDownTexID)
    , m_onOffX(argOnOffX)
    , m_onOffY(argOnOffY)
    , m_onRadius(argOnRadius)

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
              return (getState() == BEVENT_DOWN) ? g_progUseDB->retrieve(m_texID) : nullptr;
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
        if(auto texPtr = g_progUseDB->retrieve(m_texID)){
            return SDLDeviceHelper::getTextureWidth(texPtr);
        }
        return 0;
    },

    [this](const Widget *)
    {
        if(auto texPtr = g_progUseDB->retrieve(m_texID)){
            return SDLDeviceHelper::getTextureHeight(texPtr);
        }
        return 0;
    });
}
