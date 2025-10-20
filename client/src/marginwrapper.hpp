#pragma once
#include <variant>
#include <cstddef>
#include <functional>
#include <type_traits>
#include "widget.hpp"
#include "shapecropboard.hpp"

struct MarginWrapperInitArgs final
{
    Widget::VarDir dir = DIR_UPLEFT;
    Widget::VarOff x   = 0;
    Widget::VarOff y   = 0;

    Widget *wrapped           = nullptr;
    bool    wrappedAutoDelete = false;

    std::array<Widget::VarSize, 4> margin;

    std::variant<std::nullptr_t,
                 std::function<void(int, int)>,
                 std::function<void(const Widget *, int, int)>> bgDrawFunc = nullptr;

    std::variant<std::nullptr_t,
                 std::function<void(int, int)>,
                 std::function<void(const Widget *, int, int)>> fgDrawFunc = nullptr;

    std::variant<bool,
                 std::function<bool(const Widget::ROIMap &)>,
                 std::function<bool(const Widget *, const Widget::ROIMap &)>> show = true;

    Widget *parent     = nullptr;
    bool    autoDelete = false;
};

class MarginWrapper: public Widget
{
    public:
        explicit MarginWrapper(MarginWrapperInitArgs args)
            : Widget
              {{
                  .dir = std::move(args.dir),
                  .x   = std::move(args.x),
                  .y   = std::move(args.y),

                  .w = [wrapped = args.wrapped, margin = args.margin, this]{ return wrapped->w() + Widget::evalSize(margin[2], this) + Widget::evalSize(margin[3], this); },
                  .h = [wrapped = args.wrapped, margin = args.margin, this]{ return wrapped->h() + Widget::evalSize(margin[0], this) + Widget::evalSize(margin[1], this); },

                  .parent
                  {
                      .widget = args.parent,
                      .autoDelete = args.autoDelete,
                  }
              }}
        {
            // switch(argDrawFunc.index()){
            //     case 1 : Widget::addChild(new ShapeCropBoard{DIR_UPLEFT, 0, 0, [this]{ return w(); }, [this]{ return h(); }, std::move(std::get<1>(argDrawFunc)), }, true); break;
            //     case 2 : Widget::addChild(new ShapeCropBoard{DIR_UPLEFT, 0, 0, [this]{ return w(); }, [this]{ return h(); }, std::move(std::get<2>(argDrawFunc)), }, true); break;
            //     default:                                                                                                                                                    break;
            // }

            Widget::addChildAt(args.wrapped,
                    DIR_UPLEFT,

                    0, // argMargin[2],
                    0, // argMargin[0],

                    args.wrappedAutoDelete);
        }

    public:
        bool processEventDefault(const SDL_Event &event, bool valid, int startDstX, int startDstY, const Widget::ROIOpt &roi) override
        {
            return wrapped()->processEvent(event, valid, startDstX, startDstY, roi);
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
