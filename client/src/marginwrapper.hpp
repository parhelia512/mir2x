#pragma once
#include <variant>
#include <cstddef>
#include <functional>
#include <type_traits>
#include "widget.hpp"
#include "shapecropboard.hpp"

class MarginWrapper: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarOff x   = 0;
            Widget::VarOff y   = 0;

            Widget::WADPair wrapped {};
            Widget::VarMargin margin {};

            Widget::VarDrawFunc bgDrawFunc = nullptr;
            Widget::VarDrawFunc fgDrawFunc = nullptr;

            Widget::WADPair parent {};
        };

    public:
        explicit MarginWrapper(MarginWrapper::InitArgs args)
            : Widget
              {{
                  .dir = std::move(args.dir),

                  .x = std::move(args.x),
                  .y = std::move(args.y),

                  .w = [wrapped = args.wrapped.widget, margin = args.margin, this]{ return wrapped->w() + Widget::evalSize(margin[2], this) + Widget::evalSize(margin[3], this); },
                  .h = [wrapped = args.wrapped.widget, margin = args.margin, this]{ return wrapped->h() + Widget::evalSize(margin[0], this) + Widget::evalSize(margin[1], this); },

                  .parent = std::move(args.parent),
              }}
        {
            if(Widget::hasDrawFunc(args.bgDrawFunc)){
                Widget::addChild(new ShapeCropBoard
                {{
                    .w = [this]{ return w(); },
                    .h = [this]{ return h(); },

                    .drawFunc = std::move(args.bgDrawFunc),

                }}, true);
            }

            Widget::addChildAt(args.wrapped.widget,
                    DIR_UPLEFT,

                    [dx = args.margin[2], this]{ return Widget::evalSize(dx, this); },
                    [dy = args.margin[0], this]{ return Widget::evalSize(dy, this); },

                    args.wrapped.autoDelete);

            if(Widget::hasDrawFunc(args.fgDrawFunc)){
                Widget::addChild(new ShapeCropBoard
                {{
                    .w = [this]{ return w(); },
                    .h = [this]{ return h(); },

                    .drawFunc = std::move(args.fgDrawFunc),

                }}, true);
            }
        }

    public:
        bool processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m) override
        {
            return wrapped()->processEvent(event, valid, m);
        }

    public:
        auto wrapped(this auto && self) -> std::conditional_t<std::is_const_v<std::remove_reference_t<decltype(self)>>, const Widget *, Widget *>
        {
            return self.lastChild();
        }

    public:
        void addChild  (Widget *,                                                 bool) override { throw fflvalue(name()); }
        void addChildAt(Widget *, Widget::VarDir, Widget::VarOff, Widget::VarOff, bool) override { throw fflvalue(name()); }
};
