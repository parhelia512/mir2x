#include "colorf.hpp"
#include "totype.hpp"
#include "sdldevice.hpp"
#include "imageboard.hpp"

extern SDLDevice *g_sdlDevice;

ImageBoard::ImageBoard(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        Widget::VarSize argW,
        Widget::VarSize argH,

        std::function<SDL_Texture *(const Widget *)> argLoadFunc,

        bool argHFlip,
        bool argVFlip,
        int  argRotate,

        Widget::VarU32 argColor,
        Widget::VarBlendMode argBlendMode,

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

    , m_varW(std::move(argW))
    , m_varH(std::move(argH))
    , m_varColor(std::move(argColor))
    , m_varBlendMode(std::move(argBlendMode))
    , m_loadFunc(std::move(argLoadFunc))
    , m_xformPair(getHFlipRotatePair(argHFlip, argVFlip, argRotate))
{

    int texW = 0;
    int texH = 0;

    if(auto texPtr = m_loadFunc ? m_loadFunc(this) : nullptr){
        std::tie(texW, texH) = SDLDeviceHelper::getTextureSize(texPtr);
    }

    const auto varTexW = Widget::hasSize(m_varW) ? m_varW : Widget::VarSize([this](const Widget *)
    {
        if(auto texPtr = m_loadFunc ? m_loadFunc(this) : nullptr){
            return SDLDeviceHelper::getTextureWidth(texPtr);
        }
        return 0;
    });

    const auto varTexH = Widget::hasSize(m_varH) ? m_varH : Widget::VarSize([this](const Widget *)
    {
        if(auto texPtr = m_loadFunc ? m_loadFunc(this) : nullptr){
            return SDLDeviceHelper::getTextureHeight(texPtr);
        }
        return 0;
    });

    setW((m_rotate % 2 == 0) ? varTexW : varTexH);
    setH((m_rotate % 2 == 0) ? varTexH : varTexW);
}

void ImageBoard::drawEx(int dstX, int dstY, const Widget::ROIOpt &roi) const
{
    const auto roiOpt = cropDrawROI(dstX, dstY, roi);
    if(!roiOpt.has_value()){
        return;
    }

    if(!colorf::A(Widget::evalColor(m_varColor, this))){
        return;
    }

    const auto [
        drawDstX, drawDstY,
        drawSrcX, drawSrcY,
        drawSrcW, drawSrcH,

        centerOffX,
        centerOffY,

        rotateDegree] = [
            dstX,
            dstY,

            srcX = roiOpt->x,
            srcY = roiOpt->y,
            srcW = roiOpt->w,
            srcH = roiOpt->h,

            this]() -> std::array<int, 9>
    {
        // draw rotate and flip
        // all corners are indexed as 0, 1, 2, 3

        // 0      1
        //  +----+
        //  |    |   empty square means dst region before rotation
        //  +----+
        // 3      2

        // 0      1
        //  +----+
        //  |....|   solid square means dst region after rotation, the real dst region
        //  +----+
        // 3      2

        switch(m_rotate % 4){
            case 0:
                {
                    //  0           1
                    //  +-----------+
                    //  |...........|
                    //  |...........|
                    //  +-----------+
                    //  3           2

                    return
                    {
                        dstX,
                        dstY,

                        srcX,
                        srcY,

                        srcW,
                        srcH,

                        0,
                        0,

                        0,
                    };
                }
            case 1:
                {
                    // 0           1
                    // +-----------+
                    // |           |
                    // |           |
                    // o-----+-----+
                    // |.....|     2
                    // |.....|
                    // |.....|
                    // |.....|
                    // +-----+
                    // 2     1

                    return
                    {
                        dstX,
                        dstY - srcW,

                        srcY,
                        w() - srcX - srcW,

                        srcH,
                        srcW,

                        0,
                        srcW,

                        90,
                    };
                }
            case 2:
                {
                    // 0           1
                    // +-----------+
                    // |           |
                    // |           |           3
                    // +-----------o-----------+
                    // 3           |...........|
                    //             |...........|
                    //             +-----------+
                    //             1           0

                    return
                    {
                        dstX - srcW,
                        dstY - srcH,

                        w() - srcW - srcX,
                        h() - srcH - srcY,

                        srcW,
                        srcH,

                        srcW,
                        srcH,

                        180,
                    };
                }
            default:
                {
                    // 1     2
                    // +-----+
                    // |.....|
                    // |.....|
                    // |.....|           1
                    // |.....+-----------+
                    // |.....|           |
                    // |.....|           |
                    // +-----o-----------+
                    // 0     3           2

                    return
                    {
                        dstX        + srcW,
                        dstY + srcH - srcW,

                        h() - srcH - srcY,
                        srcX,

                        srcH,
                        srcW,

                        0,
                        srcW,

                        270,
                    };
                }
        }
    }();

    const auto texPtr = m_loadFunc(this);
    if(!texPtr){
        return;
    }

    const auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);

    const auto  widthRatio = to_df(texW) / ((m_rotate % 2 == 0) ? w() : h());
    const auto heightRatio = to_df(texH) / ((m_rotate % 2 == 0) ? h() : w());

    // imgSrcX
    // size and position cropped from original image, no resize, well defined

    /**/  int imgSrcX = to_dround( widthRatio * drawSrcX);
    const int imgSrcY = to_dround(heightRatio * drawSrcY);
    const int imgSrcW = to_dround( widthRatio * drawSrcW);
    const int imgSrcH = to_dround(heightRatio * drawSrcH);

    if(m_hflip){
        imgSrcX = texW - imgSrcX - imgSrcW;
    }

    const SDLDeviceHelper::EnableTextureModColor enableColor(texPtr, Widget::evalColor(m_varColor, this));
    const SDLDeviceHelper::EnableTextureBlendMode enableBlendMode(texPtr, Widget::evalBlendMode(m_varBlendMode, this));

    g_sdlDevice->drawTextureEx(
            texPtr,

            imgSrcX, imgSrcY,
            imgSrcW, imgSrcH,

            drawDstX, drawDstY,
            drawSrcW, drawSrcH,

            centerOffX,
            centerOffY,

            rotateDegree,
            m_hflip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}
