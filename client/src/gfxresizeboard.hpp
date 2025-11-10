#pragma once
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


            Widget::VarGetter<Widget *> getter = nullptr; // not-owning
            Widget::VarROI vr {};

            Widget::VarSize2D resize {};
            Widget::VarMargin margin {};

            Widget::VarDrawFunc bgDrawFunc = nullptr;
            Widget::VarDrawFunc fgDrawFunc = nullptr;

            Widget::WADPair parent {};
        };

    private:
        Widget::VarGetter<Widget *> m_getter;
        Widget::VarROI m_vr;

    private:
        Widget::VarSize2D m_resize;
        Widget::VarMargin m_margin;

    private:
        ShapeCropBoard *m_bgBoard;
        ShapeCropBoard *m_fgBoard;

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
            , m_margin(std::move(args.margin))

            , m_bgBoard(Widget::hasDrawFunc(args.bgDrawFunc) ? new ShapeCropBoard
              {{
                  .w = [this]{ return w(); },
                  .h = [this]{ return h(); },

                  .drawFunc = std::move(args.bgDrawFunc),

              }} : nullptr)

            , m_fgBoard(Widget::hasDrawFunc(args.fgDrawFunc) ? new ShapeCropBoard
              {{
                  .w = [this]{ return w(); },
                  .h = [this]{ return h(); },

                  .drawFunc = std::move(args.fgDrawFunc),

              }} : nullptr)
        {
            setSize([this]{ return margin(2) + Widget::evalSize(m_resize.w, this) + margin(3); },
                    [this]{ return margin(0) + Widget::evalSize(m_resize.h, this) + margin(1); });

            if(m_bgBoard){ Widget::addChild(m_bgBoard, true); }
            if(m_fgBoard){ Widget::addChild(m_fgBoard, true); }
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

            const auto fnOp = [gfxWidget, m = std::cref(m), &r, &func, &self](const Widget::ROI &cr, int dx, int dy, bool needDup)
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

            const int mx = self.margin(2);
            const int my = self.margin(0);
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

            if(m_bgBoard){
                drawChild(m_bgBoard, m);
            }

            gridHelper(m, [](const Widget *widget, const Widget::ROIMap &cm)
            {
                widget->draw(cm);
            });

            if(m_fgBoard){
                drawChild(m_fgBoard, m);
            }
        }

        bool processEventDefault(const SDL_Event &e, bool valid, Widget::ROIMap m) override
        {
            if(!m.calibrate(this)){
                return false;
            }

            bool takenEvent = false;
            gridHelper(m, [&e, valid, &takenEvent](Widget *widget, const Widget::ROIMap &cm)
            {
                takenEvent |= widget->processEvent(e, valid && !takenEvent, cm);
            });

            return takenEvent;
        }
};
