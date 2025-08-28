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
        template<typename IN, typename OUT_1, typename OUT_2> using check_const_cond_t         = std::conditional_t<std::is_const_v<std::remove_reference_t<IN>>, OUT_1, OUT_2>;
        template<typename IN, typename OUT                  > using check_const_cond_out_ptr_t = check_const_cond_t<IN, const OUT *, OUT *>;

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
        auto parent(this auto && self, unsigned = 1) -> check_const_cond_out_ptr_t<decltype(self), Widget>;

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
        template<typename SELF> auto foreachChild(this SELF &&, bool, std::invocable<check_const_cond_out_ptr_t<SELF, Widget>, bool> auto f) -> std::conditional_t<std::is_same_v<std::invoke_result_t<decltype(f), Widget *, bool>, bool>, bool, void>;
        template<typename SELF> auto foreachChild(this SELF &&,       std::invocable<check_const_cond_out_ptr_t<SELF, Widget>, bool> auto f) -> std::conditional_t<std::is_same_v<std::invoke_result_t<decltype(f), Widget *, bool>, bool>, bool, void>;

    private:
        void execDeath() noexcept;

    public:
        void moveFront(const Widget *);
        void moveBack (const Widget *);

    public:
        virtual void onDeath() noexcept {}

    public:
        auto firstChild(this auto && self) -> check_const_cond_out_ptr_t<decltype(self), Widget>;
        auto  lastChild(this auto && self) -> check_const_cond_out_ptr_t<decltype(self), Widget>;

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
        auto hasChild     (this auto && self, uint64_t                                 ) -> check_const_cond_out_ptr_t<decltype(self), Widget>;
        auto hasChild     (this auto && self, std::invocable<const Widget *, bool> auto) -> check_const_cond_out_ptr_t<decltype(self), Widget>;
        auto hasDescendant(this auto && self, uint64_t                                 ) -> check_const_cond_out_ptr_t<decltype(self), Widget>;
        auto hasDescendant(this auto && self, std::invocable<const Widget *, bool> auto) -> check_const_cond_out_ptr_t<decltype(self), Widget>;

    public:
        template<std::derived_from<Widget> T> auto hasParent(this auto && self) -> check_const_cond_out_ptr_t<decltype(self), T>;
};

class Widget: public WidgetTreeNode
{
    private:
        using WidgetTreeNode::check_const_cond_out_ptr_t;

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

        bool applyRootEvent(const SDL_Event &, bool);

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
        auto & data(this auto && self)
        {
            return self.m_data;
        }

    public:
        Widget *setData(std::any);

    public:
        virtual bool       in(int, int, int, int) const;
        virtual bool parentIn(int, int, int, int) const final;

    public:
        virtual bool focus() const;
        virtual Widget *setFocus(bool);
        virtual bool consumeFocus(bool, Widget * = nullptr) final;

    public:
        auto focusedChild(this auto && self) -> check_const_cond_out_ptr_t<decltype(self), Widget>;

    public:
        bool       show() const;
        bool  localShow() const;
        void   flipShow();
        Widget *setShow(Widget::VarFlag);

    public:
        bool      active() const;
        bool localActive() const;
        void  flipActive();
        Widget *setActive(Widget::VarFlag);

    public:
        void moveXTo(Widget::VarOff);
        void moveYTo(Widget::VarOff);

        void moveTo(                Widget::VarOff, Widget::VarOff);
        void moveBy(                Widget::VarOff, Widget::VarOff);
        void moveAt(Widget::VarDir, Widget::VarOff, Widget::VarOff);

    public:
        Widget *disableSetSize();

    public:
        virtual Widget *setW(Widget::VarSize) final;
        virtual Widget *setH(Widget::VarSize) final;
        virtual Widget *setSize(Widget::VarSize, Widget::VarSize) final;
};

#include "widget.impl.hpp"
