#pragma once
#include "widget.hpp"
#include "shapecropboard.hpp"

class GfxCropBoard: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            Widget::VarGetter<Widget *> getter = nullptr; // not-owning
            Widget::VarROI vr {};

            Widget::VarDrawFunc bgDrawFunc = nullptr;
            Widget::VarDrawFunc fgDrawFunc = nullptr;

            Widget::VarMargin margin {};

            Widget::InitAttrs attrs {};
            Widget::WADPair  parent {};
        };

    private:
        Widget::VarGetter<Widget *> m_getter;

    private:
        Widget::VarROI    m_vr;
        Widget::VarMargin m_margin;

    private:
        ShapeCropBoard *m_bgBoard;
        ShapeCropBoard *m_fgBoard;

    public:
        GfxCropBoard(GfxCropBoard::InitArgs args)
            : Widget
              {{
                  .dir = std::move(args.dir),
                  .x = std::move(args.x),
                  .y = std::move(args.y),

                  .attrs = std::move(args.attrs),
                  .parent = std::move(args.parent),
              }}

            , m_getter(std::move(args.getter))
            , m_vr    (std::move(args.vr    ))
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
            // respect blank space by over-cropping
            // if cropped size bigger than gfx size, it's fill with blank

            setSize([this]{ return margin(2) + gfxCropW() + margin(3); },
                    [this]{ return margin(0) + gfxCropH() + margin(1); });

            if(m_bgBoard){ Widget::addChild(m_bgBoard, true); }
            if(m_fgBoard){ Widget::addChild(m_fgBoard, true); }
        }

    private:
        void cropHelper(this auto && self, const Widget::ROIMap &m, auto && func)
        {
            if(auto gfxPtr = self.gfxWidget()){
                if(const auto r = self.gfxCropROI()){
                    func(gfxPtr, m.map(self.margin(2) - r.x, self.margin(0) - r.y, r));
                }
            }
        }

    public:
        void drawDefault(Widget::ROIMap m) const override
        {
            if(!m.calibrate(this)){
                return;
            }

            if(m_bgBoard){
                drawChild(m_bgBoard, m);
            }

            cropHelper(m, [](const auto *widget, const auto &cm)
            {
                widget->draw(cm);
            });

            if(m_fgBoard){
                drawChild(m_fgBoard, m);
            }
        }

    public:
        bool processEventDefault(const SDL_Event &e, bool valid, Widget::ROIMap m) override
        {
            if(!m.calibrate(this)){
                return false;
            }

            bool tookEvent = false;
            cropHelper(m, [&e, valid, &tookEvent](auto *widget, const auto &cm)
            {
                tookEvent = widget->processEvent(e, valid, cm);
            });

            return tookEvent;
        }

    public:
        Widget *gfxWidget() const
        {
            return Widget::evalGetter(m_getter, this);
        }

    public:
        Widget::ROI gfxCropROI() const
        {
            return m_vr.roi(this, nullptr);
        }

    public:
        int gfxCropX() const { return Widget::evalInt (m_vr.x, this); }
        int gfxCropY() const { return Widget::evalInt (m_vr.y, this); }
        int gfxCropW() const { return Widget::evalSize(m_vr.w, this); }
        int gfxCropH() const { return Widget::evalSize(m_vr.h, this); }

    public:
        int margin(int index) const
        {
            return Widget::evalSize(m_margin[index], this);
        }
};
