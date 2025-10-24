#pragma once
#include "widget.hpp"
#include "shapecropboard.hpp"

class MarginContainer: public Widget
{
    private:
        Widget::VarDir m_widgetDir;

    public:
        MarginContainer(
                Widget::VarDir argDir,
                Widget::VarOff argX,
                Widget::VarOff argY,

                Widget::VarSizeOpt argW,
                Widget::VarSizeOpt argH,

                Widget *argWidget,

                Widget::VarDir argWidgetDir,
                bool           argWidgetAutoDelete,

                std::function<void(const Widget *, int, int)> argDrawFunc = nullptr,

                Widget *argParent     = nullptr,
                bool    argAutoDelete = false)

            : Widget
              {{
                  .dir = std::move(argDir),

                  .x = std::move(argX),
                  .y = std::move(argY),
                  .w = std::move(argW),
                  .h = std::move(argH),

                  .parent
                  {
                      .widget = argParent,
                      .autoDelete = argAutoDelete,
                  }
              }}

            , m_widgetDir(std::move(argWidgetDir))
        {
            Widget::addChild(new ShapeCropBoard
            {{
                .w = [this]{ return w(); },
                .h = [this]{ return h(); },

                .drawFunc = std::move(argDrawFunc),
            }},

            true);

            Widget::addChildAt(argWidget, [this](const Widget *)
            {
                return Widget::evalDir(m_widgetDir, this);
            },

            [this](const Widget *)
            {
                switch(Widget::evalDir(m_widgetDir, this)){
                    case DIR_UPLEFT   : return       0;
                    case DIR_UP       : return w() / 2;
                    case DIR_UPRIGHT  : return w() - 1;
                    case DIR_LEFT     : return       0;
                    case DIR_RIGHT    : return w() - 1;
                    case DIR_DOWNLEFT : return       0;
                    case DIR_DOWN     : return w() / 2;
                    case DIR_DOWNRIGHT: return w() - 1;
                    default           : return w() / 2; // DIR_NONE
                }
            },

            [this](const Widget *)
            {
                switch(Widget::evalDir(m_widgetDir, this)){
                    case DIR_UPLEFT   : return       0;
                    case DIR_UP       : return       0;
                    case DIR_UPRIGHT  : return       0;
                    case DIR_LEFT     : return h() / 2;
                    case DIR_RIGHT    : return h() / 2;
                    case DIR_DOWNLEFT : return h() - 1;
                    case DIR_DOWN     : return h() - 1;
                    case DIR_DOWNRIGHT: return h() - 1;
                    default           : return h() / 2; // DIR_NONE
                }
            },

            argWidgetAutoDelete);
        }

    public:
        bool processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m) override
        {
            return contained()->processEvent(event, valid, m);
        }

    public:
        void addChild  (Widget *,                                                 bool) override { throw fflreach(); }
        void addChildAt(Widget *, Widget::VarDir, Widget::VarOff, Widget::VarOff, bool) override { throw fflreach(); }

    public:
        const Widget *contained() const { return lastChild(); }
        /* */ Widget *contained()       { return lastChild(); }
};
