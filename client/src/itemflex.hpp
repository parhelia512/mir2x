#pragma once
#include <vector>
#include <utility>
#include <initializer_list>
#include "widget.hpp"

class ItemFlex: public Widget
{
    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarOff x = 0;
            Widget::VarOff y = 0;

            Widget::VarSizeOpt length;

            bool hbox = true;
            Widget::VarSize itemSpace = 0;

            std::initializer_list<std::pair<Widget *, bool>> childList = {};
            Widget::WADPair parent {};
        };

    private:
        const bool m_hbox;

    private:
        Widget::VarSize m_itemSpace;
        std::vector<Widget *> m_origChildList;

    public:
        ItemFlex(ItemFlex::InitArgs args)
            : Widget
              {{
                  .dir = std::move(args.dir),

                  .x = std::move(args.x),
                  .y = std::move(args.y),

                  .w =  args.hbox ? Widget::VarSizeOpt{} : std::move(args.length),
                  .h = !args.hbox ? Widget::VarSizeOpt{} : std::move(args.length),

                  .parent = std::move(args.parent),
              }}

            , m_hbox(args.hbox)
            , m_itemSpace(std::move(args.itemSpace))
        {
            for(auto [widget, autoDelete]: args.childList){
                addChild(widget, autoDelete);
            }
        }

    public:
        void addChild(Widget *argWidget, bool argAutoDelete) override
        {
            m_origChildList.push_back(argWidget);
            if(m_hbox){
                addChildAt(argWidget, DIR_UPLEFT, [this](const Widget *self)
                {
                    const int itemSpace = std::max<int>(0, Widget::evalSize(m_itemSpace, this));
                    int offset = 0;

                    for(auto widget: m_origChildList){
                        if(widget == self){
                            break;
                        }

                        if(!widget->show()){
                            continue;
                        }

                        offset += widget->w();
                        offset += itemSpace;
                    }

                    return offset;
                },

                0,
                argAutoDelete);
            }
            else{
                addChildAt(argWidget, DIR_UPLEFT, 0, [this](const Widget *self)
                {
                    const int itemSpace = std::max<int>(0, Widget::evalSize(m_itemSpace, this));
                    int offset = 0;

                    for(auto widget: m_origChildList){
                        if(widget == self){
                            break;
                        }

                        if(!widget->show()){
                            continue;
                        }

                        offset += widget->h();
                        offset += itemSpace;
                    }

                    return offset;
                },

                argAutoDelete);
            }
        }

    public:
        void removeChildElement(Widget::ChildElement &argChild, bool argTrigger) override
        {
            std::erase(m_origChildList, argChild.widget);
            Widget::removeChildElement(argChild, argTrigger);
        }
};
