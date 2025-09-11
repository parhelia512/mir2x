#pragma once
#include <variant>
#include <<cstddef>
#include <functional>
#include <type_traits>
#include "widget.hpp"
#include "shapecropboard.hpp"

struct MarginWrapperInitArgs final
{
    Widget::VarDir dir;
    Widget::VarInt x;
    Widget::VarInt y;

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
        MarginWrapper(MarginWrapperInitArgs args)
            : Widget
              {{
                  .dir = std::move(args.dir),
                  .x   = std::move(args.x),
                  .y   = std::move(args.y),

                  .w = [wrapped = args.wrapped, margin = args.margin, this]{ return wrapped->w() + Widget::evalSize(margin[2], this) + Widget::evalSize(margin[3], this); },
                  .h = [wrapped = args.wrapped, margin = args.margin, this]{ return wrapped->h() + Widget::evalSize(margin[0], this) + Widget::evalSize(margin[1], this); },

                  .parent = args.parent,
                  .autoDelete = args.autoDelete,
              }}

            : Widget
              {
                  std::move(argDir),
                  std::move(argX),
                  std::move(argY),

                  [argWidget, argMargin, this]{ return argWidget->w() + Widget::evalSize(argMargin[2], this) + Widget::evalSize(argMargin[3], this); },
                  [argWidget, argMargin, this]{ return argWidget->h() + Widget::evalSize(argMargin[0], this) + Widget::evalSize(argMargin[1], this); },

                  {},

                  argParent,
                  argAutoDelete,
              }
        {
            switch(argDrawFunc.index()){
                case 1 : Widget::addChild(new ShapeCropBoard{DIR_UPLEFT, 0, 0, [this]{ return w(); }, [this]{ return h(); }, std::move(std::get<1>(argDrawFunc)), }, true); break;
                case 2 : Widget::addChild(new ShapeCropBoard{DIR_UPLEFT, 0, 0, [this]{ return w(); }, [this]{ return h(); }, std::move(std::get<2>(argDrawFunc)), }, true); break;
                default:                                                                                                                                                    break;
            }

            Widget::addChildAt(argWidget,
                    DIR_UPLEFT,

                    argMargin[2],
                    argMargin[0],

                    argWidgetAutoDelete);
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
        void addChildAt(Widget *, Widget::VarDir, Widget::VarInt, Widget::VarInt, bool) override { throw fflvalue(name()); }
};
