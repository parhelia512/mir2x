#include "colorf.hpp"
#include "sysconst.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "tritexbutton.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

TritexButton::TritexButton(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        const uint32_t (& argTexIDList)[3],
        const uint32_t (&argSeffIDList)[3],

        std::function<void(Widget *           )> argOnOverIn,
        std::function<void(Widget *           )> argOnOverOut,
        std::function<void(Widget *, bool, int)> argOnClick,
        std::function<void(Widget *      , int)> argOnTrigger,

        int argOffXOnOver,
        int argOffYOnOver,
        int argOffXOnClick,
        int argOffYOnClick,

        bool argOnClickDone,
        bool argRadioMode,
        bool argAlterColor,

        Widget *argParent,
        bool    argAutoDelete)

    : ButtonBase
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),

          .onOverIn  = std::move(argOnOverIn),
          .onOverOut = std::move(argOnOverOut),

          .onClick = std::move(argOnClick),
          .onTrigger = std::move(argOnTrigger),

          .seff
          {
              .onOverIn  = argSeffIDList[0],
              .onOverOut = argSeffIDList[1],
              .onClick   = argSeffIDList[2],
          },

          .offXOnOver = argOffXOnOver,
          .offYOnOver = argOffYOnOver,

          .offXOnClick = argOffXOnClick,
          .offYOnClick = argOffYOnClick,

          .onClickDone = argOnClickDone,
          .radioMode = argRadioMode,

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_texIDList
      {
          argTexIDList[0],
          argTexIDList[1],
          argTexIDList[2],
      }

    , m_alterColor(argAlterColor)
{
    const auto fnGetEdgeSize = [this](auto fn)
    {
        return [fn, this](const Widget *)
        {
            int result = 0;
            for(const int state: {0, 1, 2}){
                if(m_texIDList[state] != SYS_U32NIL){
                    if(auto texPtr = g_progUseDB->retrieve(m_texIDList[state])){
                        result = std::max<int>(result, fn(texPtr));
                    }
                }
            }

            // we allow buttons without any valid texture, in that case some extra work
            // can be done for special drawing
            return result;
        };
    };

    setSize(fnGetEdgeSize([](SDL_Texture *texPtr){ return SDLDeviceHelper::getTextureWidth (texPtr); }),
            fnGetEdgeSize([](SDL_Texture *texPtr){ return SDLDeviceHelper::getTextureHeight(texPtr); }));
}

void TritexButton::draw(Widget::ROIMap m) const
{
    if(!m.crop(roi())){
        return;
    }

    if(auto texPtr = g_progUseDB->retrieve(m_texIDList[getState()])){
        const int offX = m_offset[getState()][0];
        const int offY = m_offset[getState()][1];
        const auto modColor= [this]() -> uint32_t
        {
            if(!active()){
                return colorf::RGBA(128, 128, 128, 255);
            }
            else if(m_alterColor && (getState() != BEVENT_OFF)){
                return colorf::RGBA(255, 200, 255, 255);
            }
            else{
                return colorf::RGBA(255, 255, 255, 255);
            }
        }();

        const SDLDeviceHelper::EnableTextureModColor enableColor(texPtr, modColor);
        const SDLDeviceHelper::EnableTextureBlendMode enableBlendMode(texPtr, [this]()
        {
            if(m_blinkTime.has_value()){
                const auto offTime = std::get<0>(m_blinkTime.value());
                const auto  onTime = std::get<1>(m_blinkTime.value());

                if(offTime == 0){
                    return SDL_BLENDMODE_ADD;
                }
                else if(onTime == 0){
                    return SDL_BLENDMODE_BLEND;
                }
                else{
                    if(std::fmod(m_accuBlinkTime, offTime + onTime) < offTime){
                        return SDL_BLENDMODE_BLEND;
                    }
                    else{
                        return SDL_BLENDMODE_ADD;
                    }
                }
            }
            else{
                return SDL_BLENDMODE_BLEND;
            }
        }());
        g_sdlDevice->drawTexture(texPtr, m.x + offX, m.y + offY, m.ro->x, m.ro->y, m.ro->w, m.ro->h); // TODO: need to crop m.ro-> region for offset
    }
}
