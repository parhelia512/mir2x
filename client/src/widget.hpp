// class Widget has no resize(), only setSize()
// widget has no box concept like gtk, it can't calculate size in parent

#pragma once
#include <any>
#include <list>
#include <utility>
#include <vector>
#include <concepts>
#include <tuple>
#include <array>
#include <functional>
#include <variant>
#include <cstdint>
#include <optional>
#include <type_traits>
#include <initializer_list>
#include <SDL2/SDL.h>
#include "mathf.hpp"
#include "lalign.hpp"
#include "bevent.hpp"
#include "fflerror.hpp"
#include "protocoldef.hpp"

class Widget;        // size concept
class WidgetTreeNode // tree concept, used by class Widget only
{
    private:
        template<typename IN, typename OUT>
            using conditional_add_const_out_ptr_t = std::conditional_t<std::is_const_v<std::remove_reference_t<IN>>, const OUT *, OUT *>;

    protected:
        using VarDir   = std::variant<             dir8_t, std::function<  dir8_t(const Widget *)>>;
        using VarOff   = std::variant<                int, std::function<     int(const Widget *)>>;
        using VarSize  = std::variant<std::monostate, int, std::function<     int(const Widget *)>>;
        using VarFlag  = std::variant<               bool, std::function<    bool(const Widget *)>>;
        using VarColor = std::variant<           uint32_t, std::function<uint32_t(const Widget *)>>;

    private:
        friend class Widget;

    protected:
        struct ChildElement final
        {
            Widget *widget     = nullptr;
            bool    autoDelete = false;
        };

    private:
        const uint64_t m_id;

    private:
        mutable bool m_inLoop = false;

    private:
        Widget * m_parent;

    private:
        std::list<WidgetTreeNode::ChildElement> m_childList; // widget shall NOT access this list directly

    private:
        std::vector<Widget *> m_delayList;

    private:
        WidgetTreeNode(Widget * = nullptr, bool = false); // can only be constructed by Widget::Widget()

    public:
        virtual ~WidgetTreeNode();

    public:
        virtual auto parent(this auto && self, unsigned = 1) final -> conditional_add_const_out_ptr_t<decltype(self), Widget>;

    public:
        uint64_t id() const
        {
            return m_id;
        }

        const char *name() const
        {
            return typeid(*this).name();
        }

    public:
        template<std::invocable<const Widget *, bool, const Widget *, bool> F> void sort(F);

    public:
        // complicated function signature, means
        // auto foreachChild(bool, std::invocable<      Widget *, bool> auto f)       -> std::result_type_t<decltype(f),       Widget *, bool>
        // auto foreachChild(bool, std::invocable<const Widget *, bool> auto f) const -> std::result_type_t<decltype(f), const Widget *, bool>
        template<typename SELF> auto foreachChild(this SELF &&, bool, std::invocable<conditional_add_const_out_ptr_t<SELF, Widget>, bool> auto f) -> std::conditional_t<std::is_same_v<std::invoke_result_t<decltype(f), Widget *, bool>, bool>, bool, void>;
        template<typename SELF> auto foreachChild(this SELF &&,       std::invocable<conditional_add_const_out_ptr_t<SELF, Widget>, bool> auto f) -> std::conditional_t<std::is_same_v<std::invoke_result_t<decltype(f), Widget *, bool>, bool>, bool, void>;

    private:
        void execDeath() noexcept;

    public:
        void moveFront(const Widget *);
        void moveBack (const Widget *);

    public:
        virtual void onDeath() noexcept {}

    public:
        auto firstChild(this auto && self) -> conditional_add_const_out_ptr_t<decltype(self), Widget>;
        auto  lastChild(this auto && self) -> conditional_add_const_out_ptr_t<decltype(self), Widget>;

    public:
        void clearChild(std::invocable<const Widget *, bool> auto);
        void clearChild()
        {
            clearChild([](const Widget *, bool){ return true; });
        }

    protected:
        virtual void removeChildElement(WidgetTreeNode::ChildElement &, bool);

    public:
        virtual void purge();
        virtual void removeChild(Widget *, bool);

    private:
        virtual void doAddChild(Widget *, bool) final;

    public:
        virtual void addChild  (Widget *, bool);
        virtual void addChildAt(Widget *, WidgetTreeNode::VarDir, WidgetTreeNode::VarOff, WidgetTreeNode::VarOff, bool);

    public:
        bool hasChild() const
        {
            return firstChild();
        }

    public:
        auto hasChild     (this auto && self, uint64_t                                 ) -> conditional_add_const_out_ptr_t<decltype(self), Widget>;
        auto hasChild     (this auto && self, std::invocable<const Widget *, bool> auto) -> conditional_add_const_out_ptr_t<decltype(self), Widget>;
        auto hasDescendant(this auto && self, uint64_t                                 ) -> conditional_add_const_out_ptr_t<decltype(self), Widget>;
        auto hasDescendant(this auto && self, std::invocable<const Widget *, bool> auto) -> conditional_add_const_out_ptr_t<decltype(self), Widget>;

    public:
        template<std::derived_from<Widget> T> auto hasParent(this auto && self) -> conditional_add_const_out_ptr_t<decltype(self), T>;
};

class Widget: public WidgetTreeNode
{
    private:
        using WidgetTreeNode::conditional_add_const_out_ptr_t;

    public:
        using WidgetTreeNode::VarDir;
        using WidgetTreeNode::VarOff;
        using WidgetTreeNode::VarSize;
        using WidgetTreeNode::VarFlag;
        using WidgetTreeNode::VarColor;

    public:
        using WidgetTreeNode::ChildElement;

    public:
        static bool hasIntDir (const Widget::VarDir &);
        static bool hasFuncDir(const Widget::VarDir &);

        static dir8_t  asIntDir(const Widget::VarDir &);
        static dir8_t &asIntDir(      Widget::VarDir &);

        static const std::function<dir8_t(const Widget *)> &asFuncDir(const Widget::VarDir &);
        static       std::function<dir8_t(const Widget *)> &asFuncDir(      Widget::VarDir &);

        static dir8_t evalDir(const Widget::VarDir &, const Widget *);

    public:
        static bool hasIntOff (const Widget::VarOff &);
        static bool hasFuncOff(const Widget::VarOff &);

        static int  asIntOff(const Widget::VarOff &);
        static int &asIntOff(      Widget::VarOff &);

        static const std::function<int(const Widget *)> &asFuncOff(const Widget::VarOff &);
        static       std::function<int(const Widget *)> &asFuncOff(      Widget::VarOff &);

        static int evalOff(const Widget::VarOff &, const Widget *);

    public:
        static bool hasSize    (const Widget::VarSize &);
        static bool hasIntSize (const Widget::VarSize &);
        static bool hasFuncSize(const Widget::VarSize &);

        static int  asIntSize(const Widget::VarSize &);
        static int &asIntSize(      Widget::VarSize &);

        static const std::function<int(const Widget *)> &asFuncSize(const Widget::VarSize &);
        static       std::function<int(const Widget *)> &asFuncSize(      Widget::VarSize &);

    public:
        static bool hasBoolFlag(const Widget::VarFlag &);
        static bool hasFuncFlag(const Widget::VarFlag &);

        static bool  asBoolFlag(const Widget::VarFlag &);
        static bool &asBoolFlag(      Widget::VarFlag &);

        static const std::function<bool(const Widget *)> &asFuncFlag(const Widget::VarFlag &);
        static       std::function<bool(const Widget *)> &asFuncFlag(      Widget::VarFlag &);

        static bool evalFlag(const Widget::VarFlag &, const Widget *);

    public:
        static bool hasU32Color (const Widget::VarColor &);
        static bool hasFuncColor(const Widget::VarColor &);

        static uint32_t  asU32Color(const Widget::VarColor &);
        static uint32_t &asU32Color(      Widget::VarColor &);

        static const std::function<uint32_t(const Widget *)> &asFuncColor(const Widget::VarColor &);
        static       std::function<uint32_t(const Widget *)> &asFuncColor(      Widget::VarColor &);

        static uint32_t evalColor(const Widget::VarColor &, const Widget *);

    private:
        class RecursionDetector final
        {
            private:
                bool &m_flag;

            public:
                RecursionDetector(bool &flag, const char *type, const char *func)
                    : m_flag(flag)
                {
                    if(m_flag){
                        throw fflerror("recursion detected in %s::%s", type, func);
                    }
                    else{
                        m_flag = true;
                    }
                }

                ~RecursionDetector()
                {
                    m_flag = false;
                }
        };

    protected:
        std::pair<Widget::VarFlag, bool> m_show   {true, false};
        std::pair<Widget::VarFlag, bool> m_active {true, false};

    protected:
        bool m_focus  = false;

    protected:
        std::any m_data;

    private:
        Widget::VarDir m_dir;

    private:
        std::pair<Widget::VarOff, int> m_x;
        std::pair<Widget::VarOff, int> m_y;

    private:
        bool m_canSetSize = true;

    private:
        Widget::VarSize m_w;
        Widget::VarSize m_h;

    private:
        mutable bool m_hCalc = false;
        mutable bool m_wCalc = false;

    private:
        std::function<void(Widget *)> m_afterResizeHandler;
        std::function<bool(Widget *, const SDL_Event &, bool, int, int)> m_processEventHandler;

    public:
        Widget(
                Widget::VarDir,
                Widget::VarOff,
                Widget::VarOff,

                Widget::VarSize = {},
                Widget::VarSize = {},

                std::vector<std::tuple<Widget *, Widget::VarDir, Widget::VarOff, Widget::VarOff, bool>> = {},

                Widget * = nullptr,
                bool     = false);

    private:
        static int  sizeOff(   int, int);
        static int xSizeOff(dir8_t, int);
        static int ySizeOff(dir8_t, int);

    public:
        virtual void update(double);

    public:
        Widget *setProcessEvent(std::function<bool(Widget *, const SDL_Event &, bool, int, int)>);

        virtual bool processEvent      (const SDL_Event &, bool, int, int) final;
        virtual bool processParentEvent(const SDL_Event &, bool, int, int) final;

        bool applyRootEvent(const SDL_Event &);

    protected:
        // @param
        //      event
        //      valid
        //
        //      startDstX:
        //      startDstY: where the top-left corner locates when processing current event
        //
        // @return
        //      true if event is consumed
        //
        // widget has no x()/y() supported
        // when draw/processEvent, needs give explicit startDstX/startDstY when calling drawEx()/processEvent()
        // this helps to support more features, like:
        //
        //      +--------+   +--------+
        //      |        |   |        |
        //      | view_1 |   | view_2 |
        //      |        |   |        |
        //      +--------+   +--------+
        //
        // above two view-window can show the same widget, but different part of it
        // and either view can capture and process event
        virtual bool processEventDefault(const SDL_Event &, bool, int, int);

    public:
        virtual void drawEx     (                int, int, int    , int    , int     , int     ) const      ;
        virtual void drawChildEx(const Widget *, int, int, int    , int    , int     , int     ) const final;
        virtual void drawAt     (        dir8_t, int, int, int = 0, int = 0, int = -1, int = -1) const final;

        void drawRoot() const;

    public:
        Widget *setAfterResize(std::function<void(Widget *)>);

        virtual void afterResize() final;
        virtual void afterResizeDefault();

    public:
        virtual dir8_t dir() const { return Widget::evalDir(m_dir, this); }

    public:
        virtual int w() const;
        virtual int h() const;

    public:
        const auto &varw() const { return m_w; }
        const auto &varh() const { return m_h; }

    public:
        virtual int dx() const { return Widget::evalOff(m_x.first, this) + m_x.second - xSizeOff(dir(), w()); }
        virtual int dy() const { return Widget::evalOff(m_y.first, this) + m_y.second - ySizeOff(dir(), h()); }

    public:
        auto &data(this auto && self)
        {
            return m_data;
        }

        Widget *setData(std::any argData)
        {
            m_data = std::move(argData);
            return this;
        }

    public:
        virtual bool       in(int, int, int, int) const;
        virtual bool parentIn(int, int, int, int) const final;

    public:
        virtual bool focus() const;
        virtual Widget *setFocus(bool);
        virtual bool consumeFocus(bool, Widget * = nullptr) final;

    public:
        auto focusedChild(this auto && self) -> conditional_add_const_out_ptr_t<decltype(self), Widget>;

    public:
        Widget *setShow(Widget::VarFlag argShow)
        {
            m_show = std::make_pair(std::move(argShow), false);
            return this;
        }

        bool localShow() const
        {
            return Widget::evalFlag(m_show.first, this) != m_show.second;
        }

        bool show() const
        {
            // unlike active(), don't check if parent shows
            // i.e. in a item list page, we usually setup the page as auto-scaling mode to automatically updates its width/height
            //
            //  +-------------------+ <---- page
            //  | +---------------+ |
            //  | |       0       | | <---- item0
            //  | +---------------+ |
            //  | |       1       | | <---- item1
            //  | +---------------+ |
            //  |        ...        |
            //
            // when appending a new item, say item2, auto-scaling mode check current page height and append the new item at proper start:
            //
            //     page->addChild(item2, DIR_UPLEFT, 0, page->h(), true);
            //
            // if implementation of show() checks if parent shows or not
            // and if page is not shown, page->h() always return 0, even there is item0 and item1 inside
            //
            // this is possible
            // we may hide a widget before it's ready
            //
            // because item0->show() always return false, so does item1->show()
            // this makes auto-scaling fail
            //
            // we still setup m_show for each child widget
            // but when drawing, widget skips itself and all its child widgets if this->show() returns false

            if(m_parent && !m_parent->show()){
                return false;
            }
            return localShow();
        }

        void flipShow()
        {
            m_show.second = !m_show.second;
        }

    public:
        Widget *setActive(Widget::VarFlag argActive)
        {
            m_active = std::make_pair(std::move(argActive), false);
            return this;
        }

        bool localActive() const
        {
            return Widget::evalFlag(m_active.first, this) != m_active.second;
        }

        bool active() const
        {
            if(m_parent && !m_parent->active()){
                return false;
            }
            return localActive();
        }

        void flipActive()
        {
            m_active.second = !m_active.second;
        }

    public:
        void moveBy(Widget::VarOff dx, Widget::VarOff dy)
        {
            const auto fnOp = [](std::pair<Widget::VarOff, int> &offset, Widget::VarOff update)
            {
                if(Widget::hasIntOff(update)){
                    offset.second += Widget::asIntOff(update);
                }
                else if(Widget::hasIntOff(offset.first)){
                    offset.second += Widget::asIntOff(offset.first);
                    offset.first   = std::move(update);
                }
                else{
                    offset.first = [u = std::move(offset.first), v = std::move(update)](const Widget *widgetPtr)
                    {
                        return Widget::asFuncOff(u)(widgetPtr) + Widget::asFuncOff(v)(widgetPtr);
                    };
                }
            };

            fnOp(m_x, std::move(dx));
            fnOp(m_y, std::move(dy));
        }

        void moveAt(Widget::VarDir argDir, Widget::VarOff argX, Widget::VarOff argY)
        {
            m_dir = std::move(argDir);
            m_x   = std::make_pair(std::move(argX), 0);
            m_y   = std::make_pair(std::move(argY), 0);
        }

    public:
        void moveXTo(Widget::VarOff arg) { m_x   = std::make_pair(std::move(arg), 0); }
        void moveYTo(Widget::VarOff arg) { m_y   = std::make_pair(std::move(arg), 0); }

    public:
        void moveTo(Widget::VarOff argX, Widget::VarOff argY)
        {
            moveXTo(std::move(argX));
            moveYTo(std::move(argY));
        }

    public:
        Widget *disableSetSize()
        {
            m_canSetSize = true; // can not flip back
            return this;
        }

    public:
        virtual Widget *setW(Widget::VarSize argSize) final
        {
            if(m_canSetSize){
                throw fflerror("can not resize %s", name());
            }

            m_w = std::move(argSize);
            return this;
        }

        virtual Widget *setH(Widget::VarSize argSize) final
        {
            if(m_canSetSize){
                throw fflerror("can not resize %s", name());
            }

            m_h = std::move(argSize);
            return this;
        }

    public:
        virtual Widget *setSize(Widget::VarSize argW, Widget::VarSize argH) final
        {
            return setW(std::move(argW))->setH(std::move(argH));
        }
};

#include "widget.impl.hpp"
