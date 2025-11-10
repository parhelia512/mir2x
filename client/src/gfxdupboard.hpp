#pragma once
#include "widget.hpp"

class GfxDupBoard: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarSizeOpt w = 0;
            Widget::VarSizeOpt h = 0;

            Widget::VarGetter<Widget *> getter = nullptr; // not-owning
            Widget::VarROIOpt vro {};

            Widget::WADPair parent {};
        };

    private:
        Widget::VarGetter<Widget *> m_getter;
        Widget::VarROIOpt m_vro;

    public:
        GfxDupBoard(GfxDupBoard::InitArgs args)
            : Widget
              {{
                  .dir = std::move(args.dir),

                  .x = std::move(args.x),
                  .y = std::move(args.y),
                  .w = std::move(args.w),
                  .h = std::move(args.h),

                  .parent = std::move(args.parent),
              }}

            , m_getter(std::move(args.getter))
            , m_vro(std::move(args.vro))
        {}

    public:
        Widget *gfxWidget() const
        {
            return Widget::evalGetter(m_getter, this);
        }

        Widget::ROI gfxCropROI() const
        {
            if(m_vro.has_value()){
                return m_vro->roi(this, nullptr);
            }
            else if(const auto gfxPtr = gfxWidget()){
                return gfxPtr->roi();
            }
            else{
                throw fflerror("gfxCropROI");
            }
        }

    private:
        void gridHelper(this auto && self, Widget::ROIMap m, auto && func)
        {
            if(!m.calibrate(std::addressof(self))){
                return;
            }

            if(auto gfxPtr = self.gfxWidget()){
                if(const auto r = gfxPtr->roi().crop(self.gfxCropROI()); !r.empty()){
                    for(int yi = m.ro->y / r.h; yi * r.h < m.ro->y + m.ro->h; ++yi){
                        for(int xi = m.ro->x / r.w; xi * r.w < m.ro->x + m.ro->w; ++xi){
                            func(xi, yi, m.clone().crop({xi * r.w, yi * r.h, r.w, r.h}), r);
                        }
                    }
                }
            }
        }

    public:
        void drawDefault(Widget::ROIMap m) const override
        {
            gridHelper(m, [this](int xi, int yi, Widget::ROIMap sm, const Widget::ROI &r)
            {
                drawAsChild(gfxWidget(), DIR_UPLEFT, xi * r.w - r.x, yi * r.h - r.y, sm);
            });
        }

    public:
        bool processEventDefault(const SDL_Event &e, bool valid, Widget::ROIMap m) override
        {
            bool takenEvent = false;
            gridHelper(m, [this, &e, valid, &takenEvent](int xi, int yi, Widget::ROIMap sm, const Widget::ROI &r)
            {
                takenEvent |= gfxWidget()->processEvent(e, valid && !takenEvent, sm.create(Widget::ROI
                {
                    .x = xi * r.w - r.x,
                    .y = yi * r.h - r.y,
                    .w =      r.w + r.x,
                    .h =      r.h + r.y,
                }));
            });

            return takenEvent;
        }
};
