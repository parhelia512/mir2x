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

    private:
        template<typename T> using VarTypeHelper = std::variant<
            T,
            std::function<T()>,
            std::function<T(const Widget *)>,
            std::function<T(const Widget *, const void *)>>;

    private:
        using alias_VarDir       = VarTypeHelper<       dir8_t>;
        using alias_VarOff       = VarTypeHelper<          int>;
        using alias_VarU32       = VarTypeHelper<     uint32_t>;
        using alias_VarSize      = VarTypeHelper<          int>;
        using alias_VarBool      = VarTypeHelper<         bool>;
        using alias_VarBlendMode = VarTypeHelper<SDL_BlendMode>;

    protected:
        // make all var types distinct
        // this is necessary for Widget::transform
        struct VarDir      : public alias_VarDir       { using alias_VarDir      ::alias_VarDir      ; };
        struct VarOff      : public alias_VarOff       { using alias_VarOff      ::alias_VarOff      ; };
        struct VarU32      : public alias_VarU32       { using alias_VarU32      ::alias_VarU32      ; };
        struct VarSize     : public alias_VarSize      { using alias_VarSize     ::alias_VarSize     ; };
        struct VarBool     : public alias_VarBool      { using alias_VarBool     ::alias_VarBool     ; };
        struct VarBlendMode: public alias_VarBlendMode { using alias_VarBlendMode::alias_VarBlendMode; };

    protected:
        using VarSizeOpt = std::optional<VarSize>;

    private:
        friend class Widget;

    protected:
        struct WADPair final // Widget-Auto-Delete-Pair
        {
            Widget *widget     = nullptr;
            bool    autoDelete = false;
        };

    protected:
        using ChildElement = WADPair;

    private:
        const uint64_t m_id;

    private:
        mutable bool m_inLoop = false;

    private:
        Widget *m_parent;

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
        using WidgetTreeNode::VarU32;
        using WidgetTreeNode::VarSize;
        using WidgetTreeNode::VarSizeOpt;
        using WidgetTreeNode::VarBool;
        using WidgetTreeNode::VarBlendMode;

    public:
        struct ROI final
        {
            int x = 0;
            int y = 0;
            int w = 0;
            int h = 0;

            bool empty() const;
            bool in(int, int) const;
            bool overlap(const Widget::ROI &) const;

            void crop(const Widget::ROI &);
            Widget::ROI create(const Widget::ROI &) const;
        };

        class ROIOpt final
        {
            private:
                std::optional<Widget::ROI> m_roiOpt;

            public:
                ROIOpt() = default;
                ROIOpt(std::nullopt_t): ROIOpt() {};

            public:
                ROIOpt(int, int, int, int);

            public:
                ROIOpt(const Widget::ROI &);

            public:
                template<std::invocable<const Widget::ROI &> Func> auto get(Func && func, std::invoke_result_t<Func, const ROI &> def) const
                {
                    return m_roiOpt.has_value() ? func(m_roiOpt.value()) : def;
                }

            public:
                void crop(const Widget::ROI    &);
                void crop(const Widget::ROIOpt &);

                Widget::ROI    create(const Widget::ROI    &) const;
                Widget::ROIOpt create(const Widget::ROIOpt &) const;

            public:
                auto operator -> (this auto && self)
                {
                    return std::addressof(self.m_roiOpt.value());
                }

                bool has_value() const
                {
                    return m_roiOpt.has_value();
                }

                decltype(auto) value(this auto && self)
                {
                    return self.m_roiOpt.value();
                }

                Widget::ROI value_or(Widget::ROI r) const
                {
                    return m_roiOpt.value_or(r);
                }
        };

        struct ROIMap final
        {
            dir8_t dir = DIR_UPLEFT;

            int x = 0;
            int y = 0;

            Widget::ROIOpt ro = std::nullopt;

            bool empty() const
            {
                return ro.has_value() && ro->empty(); // nullopt means full region
            }

            void   crop  (const Widget::ROI &, const std::optional<Widget::ROI> & = std::nullopt);
            ROIMap create(const Widget::ROI &, const std::optional<Widget::ROI> & = std::nullopt) const;
        };

    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarOff x   = 0;
            Widget::VarOff y   = 0;

            Widget::VarSizeOpt w = std::nullopt;
            Widget::VarSizeOpt h = std::nullopt;

            std::vector<std::tuple<Widget *, Widget::VarDir, Widget::VarOff, Widget::VarOff, bool>> childList = {};

            Widget::VarBool show;
            Widget::VarBool focus;

            Widget::WADPair parent;
        };

    public:
        template<typename T> static Widget::ROI makeROI(const T &);

    public:
        using WidgetTreeNode::ChildElement;

    public:
        static dir8_t        evalDir      (const Widget::VarDir       &, const Widget *, const void * = nullptr);
        static int           evalInt      (const Widget::VarOff       &, const Widget *, const void * = nullptr);
        static uint32_t      evalU32      (const Widget::VarU32       &, const Widget *, const void * = nullptr);
        static int           evalSize     (const Widget::VarSize      &, const Widget *, const void * = nullptr);
        static bool          evalBool     (const Widget::VarBool      &, const Widget *, const void * = nullptr);
        static SDL_BlendMode evalBlendMode(const Widget::VarBlendMode &, const Widget *, const void * = nullptr);

    public:
        static int evalSizeOpt(const Widget::VarSizeOpt &, const Widget *,               const auto &);
        static int evalSizeOpt(const Widget::VarSizeOpt &, const Widget *, const void *, const auto &);

    public:
        template<typename Func> static VarSize    transform(VarSize   , Func &&);
        template<typename Func> static VarSizeOpt transform(VarSizeOpt, Func &&);

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
        std::pair<Widget::VarBool, bool> m_show   {true, false};
        std::pair<Widget::VarBool, bool> m_active {true, false};

    protected:
        bool m_focus = false;

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
        Widget::VarSizeOpt m_w;
        Widget::VarSizeOpt m_h;

    private:
        mutable bool m_hCalc = false;
        mutable bool m_wCalc = false;

    private:
        std::function<void(Widget *)> m_afterResizeHandler;
        std::function<bool(Widget *, const SDL_Event &, bool, const Widget::ROIMap &)> m_processEventHandler;

    public:
        explicit Widget(Widget::InitArgs);

    public:
        Widget(
                Widget::VarDir,
                Widget::VarOff,
                Widget::VarOff,

                Widget::VarSizeOpt,
                Widget::VarSizeOpt,

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
        Widget *setProcessEvent(std::function<bool(Widget *, const SDL_Event &, bool, const Widget::ROIMap &)>);

        virtual bool processEvent      (const SDL_Event &, bool, const Widget::ROIMap &) final;
        virtual bool processParentEvent(const SDL_Event &, bool, const Widget::ROIMap &);
        virtual bool processRootEvent  (const SDL_Event &, bool, const Widget::ROIMap &) final;

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
        //
        // for drawEx() and processEventDefault(), it doesn't check show()
        //
        //     1. if need to manually draw a widget, we ignore show() result
        //     2. if draw by its parent, parent needs to check show()
        //
        // but processEventDefault() and drawEx() needs to check if given ROI is empty
        // we agree that if a widget is not show() or ROI is empty, it
        //
        //     1. won't accept any event
        //     2. won't alter any internal state, directly bypass the event
        //     3. parent needs to change focus outside, not inside widget itself
        //
        virtual bool processEventDefault(const SDL_Event &, bool, const Widget::ROIMap &);

    public:
        virtual void drawEx(const ROIMap &) const;
        virtual void drawEx(int, int, const Widget::ROIOpt &) const; // remove later

    public:
        virtual void drawChildEx(const Widget *, int, int, const Widget::ROIOpt &               ) const final;
        virtual void drawAt     (        dir8_t, int, int, const Widget::ROIOpt & = std::nullopt) const final;

    public:
        virtual void drawAsChildEx(
                const Widget *,                         // widget as child
                dir8_t,                                 // anchor if widget as child in parent
                int,
                int,
                const Widget::ROIMap &) const final;    // roiMap of calling widget

        virtual void drawRoot(const Widget::ROIMap &) const final;

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
        int dx() const;
        int dy() const;

    public:
        virtual int rdx(const Widget * = nullptr) const final;
        virtual int rdy(const Widget * = nullptr) const final;

    public:
        Widget::ROI roi() const
        {
            return {0, 0, w(), h()};
        }

    public:
        std::optional<Widget::ROI> cropDrawROI(int &, int &, const Widget::ROIOpt &) const;
        std::optional<Widget::ROIMap> cropDrawROI(const Widget::ROIMap &) const;

    public:
        auto & data(this auto && self)
        {
            return self.m_data;
        }

    public:
        Widget *setData(std::any);

    public:
        virtual bool       in(int, int, int, int, const Widget::ROIOpt &) const;
        virtual bool parentIn(int, int, int, int, const Widget::ROIOpt &) const final;

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
        Widget *setShow(Widget::VarBool);

    public:
        bool      active() const;
        bool localActive() const;
        void  flipActive();
        Widget *setActive(Widget::VarBool);

    public:
        void moveXTo(Widget::VarOff);
        void moveYTo(Widget::VarOff);

        void moveTo(                Widget::VarOff, Widget::VarOff);
        void moveBy(                Widget::VarOff, Widget::VarOff);
        void moveAt(Widget::VarDir, Widget::VarOff, Widget::VarOff);

    public:
        Widget *disableSetSize();

    public:
        virtual Widget *setW(Widget::VarSizeOpt) final;
        virtual Widget *setH(Widget::VarSizeOpt) final;
        virtual Widget *setSize(Widget::VarSizeOpt, Widget::VarSizeOpt) final;
};

#include "widget.impl.hpp"
