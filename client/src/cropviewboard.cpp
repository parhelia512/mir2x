#include "widget.hpp"
#include "cropviewboard.hpp"

CropViewBoard::CropViewBoard(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        VarGfxWidget argWidgetGetter,

        Widget::VarOff argGfxCropX, // crop on gfx widget
        Widget::VarOff argGfxCropY, // ...
        Widget::VarOff argGfxCropW, // crop width, don't use Widget::VarSize, support over-cropping
        Widget::VarOff argGfxCropH, // ...

        std::array<Widget::VarOff, 4> argMargin,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),
          0,
          0,

          {},

          argParent,
          argAutoDelete,
      }

    , m_gfxWidgetGetter(std::move(argWidgetGetter))

    , m_gfxCropX(std::move(argGfxCropX))
    , m_gfxCropY(std::move(argGfxCropY))
    , m_gfxCropW(std::move(argGfxCropW))
    , m_gfxCropH(std::move(argGfxCropH))

    , m_margin(std::move(argMargin))
{
    // respect blank space by over-cropping
    // if cropped size bigger than gfx size, it's fill with blank

    setSize([this](const Widget *)
    {
        return gfxCropW() + margin(2) + margin(3);
    },

    [this](const Widget *)
    {
        return gfxCropH() + margin(0) + margin(1);
    });
}

void CropViewBoard::drawEx(int dstX, int dstY, const Widget::ROIOpt &roi) const
{
    if(!show()){
        return;
    }

    const auto gfxPtr = gfxWidget();
    if(!gfxPtr){
        return;
    }

    const auto srcROI = roi.evalROI(this);
    if(srcROI.empty()){
        return;
    }

    int srcX = srcROI.x;
    int srcY = srcROI.y;
    int srcW = srcROI.w;
    int srcH = srcROI.h;

    if(!mathf::cropViewROI(
                &srcX, &srcY,
                &srcW, &srcH,
                &dstX, &dstY,

                margin(2),
                margin(0),

                w(),
                h(),

                gfxCropX(),
                gfxCropY(),
                gfxCropW(),
                gfxCropH(),

                gfxPtr->w(),
                gfxPtr->h())){
        return;
    }
    gfxPtr->drawEx(dstX, dstY, {srcX, srcY, srcW, srcH});
}

bool CropViewBoard::processEventDefault(const SDL_Event &event, bool valid, int startDstX, int startDstY, const Widget::ROIOpt &roi)
{
    if(!show()){
        return false;
    }

    const auto gfxPtr = gfxWidget();
    if(!gfxPtr){
        return false;
    }

    const auto srcROI = roi.evalROI(this);
    if(srcROI.empty()){
        return false;
    }

    int srcX = srcROI.x;
    int srcY = srcROI.y;
    int srcW = srcROI.w;
    int srcH = srcROI.h;

    if(!mathf::cropViewROI(
                &srcX, &srcY,
                &srcW, &srcH,

                &startDstX,
                &startDstY,

                margin(2),
                margin(0),

                w(),
                h(),

                gfxCropX(),
                gfxCropY(),
                gfxCropW(),
                gfxCropH(),

                gfxPtr->w(),
                gfxPtr->h())){
        return false;
    }

    return gfxPtr->processEvent(event, valid, startDstX, startDstY, {srcX, srcY, srcW, srcH});
}
