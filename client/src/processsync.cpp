#include "log.hpp"
#include "client.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "fontstyle.hpp"
#include "processsync.hpp"

extern Client *g_client;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

ProcessSync::ProcessSync()
    : Process()
    , m_canvas
      {
          DIR_UPLEFT,
          0,
          0,
          800,
          600,
      }

    , m_barFull
      {
          DIR_UPLEFT,
          0,
          0,

          {},
          {},
          [](const Widget *){ return g_progUseDB->retrieve(0X00000002); },
      }

    , m_bar
      {
          DIR_UPLEFT,
          112,
          528,

          &m_barFull,

          0,
          0,
          [this]{ return m_barFull.w() * m_ratio / 100; },
          [this]{ return m_barFull.h()                ; },

          {},

          &m_canvas,
          false,
      }

    , m_bgImg
      {
          DIR_UPLEFT,
          0,
          0,

          {},
          {},
          [](const Widget *){ return g_progUseDB->retrieve(0X00000001); },

          false,
          false,
          0,

          colorf::WHITE_A255,
          SDL_BLENDMODE_BLEND,

          &m_canvas,
          false,
      }

    , m_barText
      {
          DIR_NONE,
          400,
          536,

          [](const Widget *){ return "Connecting..."; },

          1,
          10,
          0,

          colorf::WHITE_A255,
          SDL_BLENDMODE_BLEND,

          &m_canvas,
          false,
      }
{}

void ProcessSync::processEvent(const SDL_Event &event)
{
    switch(event.type){
        case SDL_KEYDOWN:
            {
                if(event.key.keysym.sym == SDLK_ESCAPE){
                    g_client->requestProcess(PROCESSID_LOGIN);
                }
                break;
            }
        default:
            {
                break;
            }
    }
}

void ProcessSync::update(double fUpdateTime)
{
    if(m_ratio >= 100){
        g_client->requestProcess(PROCESSID_LOGIN);
        return;
    }

    m_ratio += (fUpdateTime > 0.0 ? 1 : 0);
}

void ProcessSync::draw() const
{
    const SDLDeviceHelper::RenderNewFrame newFrame;
    m_canvas.drawRoot({});
}
