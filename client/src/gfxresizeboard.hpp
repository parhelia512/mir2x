#pragma once
#include <initializer_list>
#include "widget.hpp"
#include "gfxdupboard.hpp"
#include "gfxcropboard.hpp"

class GfxResizeBoard: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarMargin margin {};

            Widget::VarGetter<Widget *> getter = nullptr; // not-owning
            Widget::VarROI vr {};

            Widget::VarSize2D resize {};
            Widget::WADPair parent {};
        };

    private:
        Widget::VarGetter<Widget *> m_getter;
        Widget::VarROI m_vr;

    private:
        Widget::VarSize2D m_resize;

    public:
        GfxResizeBoard(GfxResizeBoard::InitArgs args)
            : Widget
              {{
                  .dir = std::move(args.dir),

                  .x = std::move(args.x),
                  .y = std::move(args.y),

                  .parent = std::move(args.parent),
              }}

            , m_getter(std::move(args.getter))
            , m_vr(std::move(args.vr))
            , m_resize(std::move(args.resize))
        {
            setSize([this]

        }

    public:
        Widget *gfxWidget() const
        {
            return Widget::evalGetter(m_getter, this);
        }

        Widget::ROI gfxCropROI() const
        {
            return m_vr.roi(this, nullptr);
        }

    public:
        int margin(int index) const
        {
            return Widget::evalSize(m_margin[index], this);
        }

    private:
        void gridHelper(this auto && self, Widget::ROIMap m, auto && func)
        {
            auto gfxWidget = self.gfxWidget();
            if(!gfxWidget){
                return;
            }

            const auto r = self.gfxCropROI();
            if(!r){
                return;
            }

            const auto fnOp = [m = std::cref(m), &func](const Widget::ROI &cr, int dx, int dy, bool needDup)
            {
                if(!cr){
                    return;
                }

                if(const auto cm = m.get().map(dx, dy, cr)){
                    GfxCropBoard crop{{.getter = gfxWidget, .roi = cr}};
                    if(needDup){
                        GfxDupBoard dup{{.w = cr.w, .h = cr.h, .getter = &crop}};
                        func(&dup, cm);
                    }
                    else{
                        func(&crop, cm);
                    }
                }
            };

            const int mx = margin(2);
            const int my = margin(0);
            const int cw = gfxWidget->w();
            const int ch = gfxWidget->h();
            const int rw = Widget::evalSize(self.m_resize.w, &self);
            const int rh = Widget::evalSize(self.m_resize.h, &self);

            fnOp({        0,         0,            r.x,            r.y}, mx           , my           , false); // top-left
            fnOp({      r.x,         0,            r.w,            r.y}, mx + r.x     , my           , true ); // top-middle
            fnOp({r.x + r.w,         0, cw - r.x - r.w,            r.y}, mx + r.x + rw, my           , false); // top-right
            fnOp({        0,       r.y,            r.x,            r.h}, mx           , my + r.y     , true ); // middle-left
            fnOp({      r.x,       r.y,            r.w,            r.h}, mx + r.x     , my + r.y     , true ); // middle
            fnOp({r.x + r.w,       r.y, cw - r.x - r.w,            r.h}, mx + r.x + rw, my + r.y     , true ); // middle-right
            fnOp({        0, r.y + r.h,            r.x, ch - r.y - r.h}, mx           , my + r.y + rh, false); // bottom-left
            fnOp({      r.x, r.y + r.h,            r.w, ch - r.y - r.h}, mx + r.x     , my + r.y + rh, true ); // bottom-middle
            fnOp({r.x + r.w, r.y + r.h, cw - r.x - r.w, ch - r.y - r.h}, mx + r.x + rw, my + r.y + rh, false); // bottom-right
        }

    public:
        void draw(Widget::ROIMap m) const override
        {
            if(!m.calibrate(this)){
                return;
            }

            gridHelper(m, [](const Widget *widget, const Widget::ROIMap &cm)
            {
                wiget->draw(cm);
            });
        }

        bool processEventDefalut(const SDL_Event &e, bool valid, Widget::ROIMap m) override
        {
            if(!m.calibrate(this)){
                return false;
            }

            gridHelper(*this, m, [&e, &valid](Widget *widget, const Widget::ROIMap &cm)
            {
                valid = !widget->processEvent(e, valid, cm);
            });

            return !valid;
        }
};
