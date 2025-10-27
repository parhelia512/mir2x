#include "widget.hpp"
#include "cropviewboard.hpp"

CropViewBoard::CropViewBoard(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        VarGfxWidget argWidgetGetter,

        Widget::VarOff argGfxCropX, // crop on gfx widget
        Widget::VarOff argGfxCropY, // ...
        Widget::VarOff argGfxCropW, // crop width, don't use Widget::VarSizeOpt, support over-cropping
        Widget::VarOff argGfxCropH, // ...

        std::array<Widget::VarOff, 4> argMargin,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = std::move(argDir),
          .x = std::move(argX),
          .y = std::move(argY),
          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

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

void CropViewBoard::draw(Widget::ROIMap m) const
{
    if(!show()){
        return;
    }

    const auto gfxPtr = gfxWidget();
    if(!gfxPtr){
        return;
    }

    if(!m.calibrate(this)){
        return;
    }

    int dstX = m.x;
    int dstY = m.y;
    int srcX = m.ro->x;
    int srcY = m.ro->y;
    int srcW = m.ro->w;
    int srcH = m.ro->h;

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
    gfxPtr->draw({.x{dstX}, .y{dstY}, .ro{srcX, srcY, srcW, srcH}});
}

bool CropViewBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!show()){
        return false;
    }

    const auto gfxPtr = gfxWidget();
    if(!gfxPtr){
        return false;
    }

    if(!m.calibrate(this)){
        return false;
    }

    int dstX = m.x;
    int dstY = m.y;
    int srcX = m.ro->x;
    int srcY = m.ro->y;
    int srcW = m.ro->w;
    int srcH = m.ro->h;

    if(!mathf::cropViewROI(
                &srcX, &srcY,
                &srcW, &srcH,

                &dstX,
                &dstY,

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

    return gfxPtr->processEvent(event, valid, {.x{dstX}, .y{dstY}, .ro{srcX, srcY, srcW, srcH}});
}
