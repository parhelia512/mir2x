#include "colorf.hpp"
#include "totype.hpp"
#include "bevent.hpp"
#include "sdldevice.hpp"
#include "alphaonbutton.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

AlphaOnButton::AlphaOnButton(
        Widget::VarDir argDir,
        Widget::VarInt argX,
        Widget::VarInt argY,

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
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),
          0,
          0,

          std::move(fnOnOverIn),
          std::move(fnOnOverOut),
          std::move(fnOnClick),
          std::move(fnOnTrigger),

          SYS_U32NIL,
          SYS_U32NIL,
          SYS_U32NIL,

          0,
          0,
          0,
          0,

          triggerOnDone,
          false,

          pwidget,
          autoDelete,
      }

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
