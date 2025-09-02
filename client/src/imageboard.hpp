#pragma once
#include <functional>
#include <SDL2/SDL.h>
#include "widget.hpp"
#include "colorf.hpp"

// check drawTextureEx comments for how image flip/rotation works
//
//      ---H-->
//
//    x--y | y--x
//    |  | | |  |    |    Vflip = Hflip + 180
//    +--+ | +--+    |    Hflip = Vflip + 180
// --------+-------  V    Hflip + Vflip = 180
//    +--+ | +--+    |
//    |  | | |  |    v
//    x--y | y--x

// top-left corners of image and widget are aligned
// widget uses original image width/height if widget size is given by {}, otherwise rescaling applied
class ImageBoard: public Widget
{
    private:
        Widget::VarSizeOpt m_varW;
        Widget::VarSizeOpt m_varH;

    private:
        Widget::VarU32 m_varColor;
        Widget::VarBlendMode m_varBlendMode;

    private:
        std::function<SDL_Texture *(const Widget *)> m_loadFunc;

    private:
        std::pair<bool, int> m_xformPair;

    private:
        bool &m_hflip  = m_xformPair.first;
        int  &m_rotate = m_xformPair.second;

    public:
        ImageBoard(
                Widget::VarDir,
                Widget::VarInt,
                Widget::VarInt,

                Widget::VarSizeOpt, // {} means image width , otherwise rescale the image
                Widget::VarSizeOpt, // {} means image height, otherwise rescale the image

                std::function<SDL_Texture *(const Widget *)>,

                bool = false,
                bool = false,
                int  = 0,

                Widget::VarU32       = colorf::WHITE + colorf::A_SHF(0XFF),
                Widget::VarBlendMode = SDL_BLENDMODE_NONE,

                Widget * = nullptr,
                bool     = false);

    public:
        void drawEx(int, int, const Widget::ROIOpt &) const override;

    public:
        void setColor(Widget::VarU32 color)
        {
            m_varColor = std::move(color);
        }

        void setLoadFunc(std::function<SDL_Texture *(const Widget *)> func)
        {
            m_loadFunc = std::move(func);
        }

    public:
        void setFlipRotate(bool hflip, bool vflip, int rotate)
        {
            m_xformPair = getHFlipRotatePair(hflip, vflip, rotate);
        }

    private:
        static std::pair<bool, int> getHFlipRotatePair(bool hflip, bool vflip, int rotate)
        {
            if     (hflip && vflip) return {false, (((rotate + 2) % 4) + 4) % 4};
            else if(hflip         ) return { true, (((rotate    ) % 4) + 4) % 4};
            else if(         vflip) return { true, (((rotate + 2) % 4) + 4) % 4};
            else                    return {false, (((rotate    ) % 4) + 4) % 4};
        }

    public:
        SDL_Texture *getTexture() const
        {
            return m_loadFunc ? m_loadFunc(this) : nullptr;
        }
};
