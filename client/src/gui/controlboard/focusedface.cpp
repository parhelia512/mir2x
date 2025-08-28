#include "focusedface.hpp"

FocusedFace::FocusedFace(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        Widget::VarSize argW,
        Widget::VarSize argH,

        ProcessRun *argProc,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
      }

    , m_face
      {
          DIR_UPLEFT,
          2,
          3,

          {},
          {},

          [](const Widget *)
          {

          }

          false,
          false,
          0,

          colorf::WHITE + colorf::A_SHF(255),

          this,
          false,
      }

    , m_hpBar
      {
          DIR_UPLEFT,
          0,
          0,

          [this](const Widget *) { return to_dround(getHPRatio() * 87); },
          {},

          [](const Widget *)
          {
              return g_progUseDB->retrieve(0X00000015);
          }

          false,
          false,
          0,

          colorf::WHITE + colorf::A_SHF(255),

          this,
          false,
      }

    , m_drawBufIDList
      {
          DIR_UPLEFT,
          0,
          0,

          [this](const Widget *){ return w(); },
          [this](const Widget *){ return h(); },

          [this](const Widget *self, int drawDstX, int drawDstY)
          {
              drawBufIDList(drawDstX, drawDstY, self->w(), self->h());
          }
      }
{}

void FocusedFace::drawBufIDList(int drawDstX, int drawDstY, int drawDstW, int drawDstH)
{
    const auto sdBufIDListOpt = getBufIDListOpt();
    if(!sdBufIDListOpt.has_value()){
        return;
    }

    const auto &sdBufIDList = sdBufIDListOpt.value();

    constexpr int buffIconDrawW = 16;
    constexpr int buffIconDrawH = 16;

    // +--16--+
    // |      |
    // |      16
    // |      |
    // *------+
    // ^
    // |
    // +--- (buffIconOffStartX, buffIconOffStartY)

    const int buffIconOffStartX = nW0 - 266;
    const int buffIconOffStartY = nY0 +  99;

    for(int drawIconCount = 0; const auto id: buffIDList.value().idList){
        const auto &br = DBCOM_BUFFRECORD(id);
        fflassert(br);

        if(br.icon.gfxID != SYS_U32NIL){
            if(auto iconTexPtr = g_progUseDB->retrieve(br.icon.gfxID)){
                const int buffIconOffX = buffIconOffStartX + (drawIconCount % 5) * buffIconDrawW;
                const int buffIconOffY = buffIconOffStartY - (drawIconCount / 5) * buffIconDrawH;

                const auto [texW, texH] = SDLDeviceHelper::getTextureSize(iconTexPtr);
                g_sdlDevice->drawTexture(iconTexPtr, buffIconOffX, buffIconOffY, buffIconDrawW, buffIconDrawH, 0, 0, texW, texH);

                const auto baseColor = [&br]() -> uint32_t
                {
                    if(br.favor > 0){
                        return colorf::GREEN;
                    }
                    else if(br.favor == 0){
                        return colorf::YELLOW;
                    }
                    else{
                        return colorf::RED;
                    }
                }();

                const auto startColor = baseColor | colorf::A_SHF(255);
                const auto   endColor = baseColor | colorf::A_SHF( 64);

                const auto edgeGridCount = (buffIconDrawW + buffIconDrawH) * 2 - 4;
                const auto startLoc = std::lround(edgeGridCount * std::fmod(m_accuTime, 1500.0) / 1500.0);

                g_sdlDevice->drawBoxFading(startColor, endColor, buffIconOffX, buffIconOffY, buffIconDrawW, buffIconDrawH, startLoc, buffIconDrawW + buffIconDrawH);
                drawIconCount++;
            }
        }
    }
}
