#include <atomic>
#include "mathf.hpp"
#include "pathf.hpp"
#include "widget.hpp"

WidgetTreeNode::WidgetTreeNode(Widget *argParent, bool argAutoDelete)
    : m_id([]
      {
          static std::atomic<uint64_t> s_widgetSeqID = 1;
          return s_widgetSeqID.fetch_add(1);
      }())
    , m_parent(argParent)
{
    if(m_parent){
        m_parent->addChild(static_cast<Widget *>(this), argAutoDelete);
    }
}

WidgetTreeNode::~WidgetTreeNode()
{
    // if construct a widget failed
    // destructor will be called, we need to automatically remove it from parent

    // otherwise later parent destructor will try to call all its children's destructor again
    // which causes double free

    if(m_parent){
        m_parent->removeChild(static_cast<Widget *>(this), false);
    }

    clearChild();
    for(auto widget: m_delayList){
        delete widget;
    }
}

void WidgetTreeNode::moveFront(const Widget *widget)
{
    if(m_inLoop){
        throw fflerror("can not modify child list while in loop");
    }

    auto pivot = std::find_if(m_childList.begin(), m_childList.end(), [widget](const auto &x)
    {
        return x.widget == widget;
    });

    if(pivot == m_childList.end()){
        throw fflerror("can not find child widget");
    }

    std::rotate(m_childList.begin(), pivot, std::next(pivot));
}

void WidgetTreeNode::moveBack(const Widget *widget)
{
    if(m_inLoop){
        throw fflerror("can not modify child list while in loop");
    }

    auto pivot = std::find_if(m_childList.begin(), m_childList.end(), [widget](const auto &x)
    {
        return x.widget == widget;
    });

    if(pivot == m_childList.end()){
        throw fflerror("can not find child widget");
    }

    std::rotate(pivot, std::next(pivot), m_childList.end());
}

void WidgetTreeNode::execDeath() noexcept
{
    for(auto &child: m_childList){
        if(child.widget){
            child.widget->execDeath();
        }
    }
    onDeath();
}

void WidgetTreeNode::doAddChild(Widget *argWidget, bool argAutoDelete)
{
    fflassert(argWidget);
    WidgetTreeNode *treeNode = argWidget;

    if(treeNode->m_parent){
        treeNode->m_parent->removeChild(argWidget, false);
    }

    treeNode->m_parent = static_cast<Widget *>(this);
    m_childList.emplace_back(argWidget, argAutoDelete);
}

void WidgetTreeNode::addChild(Widget *argWidget, bool argAutoDelete)
{
    doAddChild(argWidget, argAutoDelete);
}

void WidgetTreeNode::addChildAt(Widget *argWidget, WidgetTreeNode::VarDir argDir, WidgetTreeNode::VarOff argX, WidgetTreeNode::VarOff argY, bool argAutoDelete)
{
    doAddChild(argWidget, argAutoDelete);
    argWidget->moveAt(std::move(argDir), std::move(argX), std::move(argY));
}

void WidgetTreeNode::removeChild(Widget *argWidget, bool argTriggerDelete)
{
    if(argWidget){
        for(auto p = m_childList.begin(); p != m_childList.end(); ++p){
            if(p->widget == argWidget){
                removeChildElement(*p, argTriggerDelete);
                return;
            }
        }
    }
}

void WidgetTreeNode::removeChildElement(WidgetTreeNode::ChildElement &argElement, bool argTriggerDelete)
{
    if(auto widptr = argElement.widget){
        widptr->m_parent = nullptr;
        argElement.widget = nullptr;

        if(argElement.autoDelete && argTriggerDelete){
            widptr->execDeath();
            m_delayList.push_back(widptr);
        }
    }
}

void WidgetTreeNode::purge()
{
    if(m_inLoop){
        throw fflerror("can not modify child list while in loop");
    }

    foreachChild([](Widget *widget, bool)
    {
        widget->purge();
    });

    for(auto widget: m_delayList){
        delete widget;
    }

    m_delayList.clear();
    m_childList.remove_if([](const auto &x) -> bool
    {
        return x.widget == nullptr;
    });
}

bool Widget::ROI::empty() const
{
    return (w <= 0) || (h <= 0);
}

bool Widget::ROI::in(int argX, int argY) const
{
    return mathf::pointInRectangle<int>(argX, argY, x, y, w, h);
}

bool Widget::ROI::crop(const Widget::ROI &r)
{
    const bool cropx = mathf::cropSegment<int>(x, w, r.x, r.w);
    const bool cropy = mathf::cropSegment<int>(y, h, r.y, r.h);

    return cropx && cropy;
}

bool Widget::ROI::overlap(const Widget::ROI &r) const
{
    return mathf::rectangleOverlap(x, y, w, h, r.x, r.y, r.w, r.h);
}

Widget::ROIOpt::ROIOpt(int argW, int argH)
    : Widget::ROIOpt::ROIOpt(0, 0, argW, argH)
{}

Widget::ROIOpt::ROIOpt(int argX, int argY, int argW, int argH)
    : m_roiOpt(Widget::ROI{argX, argY, argW, argH})
{}

Widget::ROIOpt::ROIOpt(const Widget::ROI &roi)
    : m_roiOpt(roi)
{}

bool Widget::ROIMap::calibrate(const Widget *widget)
{
    if(!ro.has_value()){
        if(widget){
            ro = widget->roi();
        }
        else{
            throw fflerror("invalid widget");
        }
    }

    if(dir != DIR_UPLEFT){
        x  -= xSizeOff(dir, ro->w);
        y  -= ySizeOff(dir, ro->h);
        dir = DIR_UPLEFT;
    }

    if(widget){
        crop(widget->roi());
    }

    if(x < 0){
        ro->x -= x;
        ro->w  = std::max<int>(ro->w + x, 0);
        x = 0;
    }

    if(y < 0){
        ro->y -= y;
        ro->h  = std::max<int>(ro->h + y, 0);
        y = 0;
    }

    return !ro->empty();
}

bool Widget::ROIMap::crop(const Widget::ROI &r)
{
    if(!ro.has_value()){
        throw fflerror("ro empty");
    }

    if(dir != DIR_UPLEFT){
        x  -= xSizeOff(dir, ro->w);
        y  -= ySizeOff(dir, ro->h);
        dir = DIR_UPLEFT;
    }

    const auto oldX = ro->x;
    const auto oldY = ro->y;

    ro->crop(r);

    x += (ro->x - oldX);
    y += (ro->y - oldY);

    return !empty();
}

Widget::ROIMap Widget::ROIMap::create(const Widget::ROI &cr /* child ROI */) const
{
    auto r = *this;
    r.crop(cr);

    r.ro->x -= cr.x;
    r.ro->y -= cr.y;

    return r;
}

bool Widget::ROIMap::empty() const
{
    if(ro.has_value()){
        return ro->empty();
    }
    else{
        throw fflerror("ro empty");
    }
}

bool Widget::ROIMap::in(int pixelX, int pixelY) const
{
    if(ro.has_value()){
        return Widget::ROI{x, y, ro->w, ro->h}.in(pixelX, pixelY);
    }
    else{
        throw fflerror("ro empty");
    }
}

dir8_t Widget::evalDir(const Widget::VarDir &varDir, const Widget *widget, const void *arg)
{
    const auto fnValidDir = [](dir8_t argDir)
    {
        return pathf::dirValid(argDir) ? argDir : DIR_NONE;
    };

    return std::visit(VarDispatcher
    {
        [&fnValidDir](dir8_t varg)
        {
            return fnValidDir(varg);
        },

        [&fnValidDir](const std::function<dir8_t()> &varg)
        {
            return varg ? fnValidDir(varg()) : DIR_NONE;
        },

        [&fnValidDir, widget](const std::function<dir8_t(const Widget *)> &varg)
        {
            return varg ? fnValidDir(varg(widget)) : DIR_NONE;
        },

        [&fnValidDir, widget, arg](const std::function<dir8_t(const Widget *, const void *)> &varg)
        {
            return varg ? fnValidDir(varg(widget, arg)) : DIR_NONE;
        },
    },

    varDir);
}

int Widget::evalInt(const Widget::VarOff &varOffset, const Widget *widget, const void *arg)
{
    return std::visit(VarDispatcher
    {
        [](int varg)
        {
            return varg;
        },

        [](const std::function<int()> &varg)
        {
            return varg ? varg() : 0;
        },

        [widget](const std::function<int(const Widget *)> &varg)
        {
            return varg ? varg(widget) : 0;
        },

        [widget, arg](const std::function<int(const Widget *, const void *)> &varg)
        {
            return varg ? varg(widget, arg) : 0;
        },
    },

    varOffset);
}

uint32_t Widget::evalU32(const Widget::VarU32 &varU32, const Widget *widget, const void *arg)
{
    return std::visit(VarDispatcher
    {
        [](uint32_t varg)
        {
            return varg;
        },

        [](const std::function<uint32_t()> &varg)
        {
            return varg ? varg() : 0;
        },

        [widget](const std::function<uint32_t(const Widget *)> &varg)
        {
            return varg ? varg(widget) : 0;
        },

        [widget, arg](const std::function<uint32_t(const Widget *, const void *)> &varg)
        {
            return varg ? varg(widget, arg) : 0;
        },
    },

    varU32);
}

int Widget::evalSize(const Widget::VarSize &varSize, const Widget *widget, const void *arg)
{
    return std::visit(VarDispatcher
    {
        [](int varg)
        {
            return std::max<int>(0, varg);
        },

        [](const std::function<int()> &varg)
        {
            return varg ? std::max<int>(0, varg()) : 0;
        },

        [widget](const std::function<int(const Widget *)> &varg)
        {
            return varg ? std::max<int>(0, varg(widget)) : 0;
        },

        [widget, arg](const std::function<int(const Widget *, const void *)> &varg)
        {
            return varg ? std::max<int>(0, varg(widget, arg)) : 0;
        },
    },

    varSize);
}

bool Widget::evalBool(const Widget::VarBool &varFlag, const Widget *widget, const void *arg)
{
    return std::visit(VarDispatcher
    {
        [](bool varg)
        {
            return varg;
        },

        [](const std::function<bool()> &varg)
        {
            return varg ? varg() : false;
        },

        [widget](const std::function<bool(const Widget *)> &varg)
        {
            return varg ? varg(widget) : false;
        },

        [widget, arg](const std::function<bool(const Widget *, const void *)> &varg)
        {
            return varg ? varg(widget, arg) : false;
        },
    },

    varFlag);
}

SDL_BlendMode Widget::evalBlendMode(const Widget::VarBlendMode &varBlendMode, const Widget *widget, const void *arg)
{
    const auto fnValidMode = [](SDL_BlendMode argMode)
    {
        switch(argMode){
            case SDL_BLENDMODE_ADD:
            case SDL_BLENDMODE_MOD:
            case SDL_BLENDMODE_MUL:
            case SDL_BLENDMODE_BLEND: return argMode;
            default                 : return SDL_BLENDMODE_NONE;
        }
    };

    return std::visit(VarDispatcher
    {
        [&fnValidMode](SDL_BlendMode varg)
        {
            return fnValidMode(varg);
        },

        [&fnValidMode](const std::function<SDL_BlendMode()> &varg)
        {
            return varg ? fnValidMode(varg()) : SDL_BLENDMODE_NONE;
        },

        [&fnValidMode, widget](const std::function<SDL_BlendMode(const Widget *)> &varg)
        {
            return varg ? fnValidMode(varg(widget)) : SDL_BLENDMODE_NONE;
        },

        [&fnValidMode, widget, arg](const std::function<SDL_BlendMode(const Widget *, const void *)> &varg)
        {
            return varg ? fnValidMode(varg(widget, arg)) : SDL_BLENDMODE_NONE;
        },
    },

    varBlendMode);
}

std::string Widget::evalStrFunc(const Widget::VarStrFunc &varStrFunc, const Widget *widget, const void *arg)
{
    return std::visit(VarDispatcher
    {
        [](const std::string &varg)
        {
            return varg;
        },

        [](const std::function<std::string()> &varg)
        {
            return varg ? varg() : std::string();
        },

        [widget](const std::function<std::string(const Widget *)> &varg)
        {
            return varg ? varg(widget) : std::string();
        },

        [widget, arg](const std::function<std::string(const Widget *, const void *)> &varg)
        {
            return varg ? varg(widget, arg) : std::string();
        },
    },

    varStrFunc);
}

SDL_Texture *Widget::evalTexLoadFunc(const Widget::VarTexLoadFunc &varTexLoadFunc, const Widget *widget, const void *arg)
{
    return std::visit(VarDispatcher
    {
        [](SDL_Texture *varg)
        {
            return varg;
        },

        [](const std::function<SDL_Texture *()> &varg)
        {
            return varg ? varg() : nullptr;
        },

        [widget](const std::function<SDL_Texture *(const Widget *)> &varg)
        {
            return varg ? varg(widget) : nullptr;
        },

        [widget, arg](const std::function<SDL_Texture *(const Widget *, const void *)> &varg)
        {
            return varg ? varg(widget, arg) : nullptr;
        },
    },

    varTexLoadFunc);
}

bool Widget::hasDrawFunc(const Widget::VarDrawFunc &varDrawFunc)
{
    return std::visit(VarDispatcher
    {
        [](const std::function<void(                        int, int)> &varg) -> bool { return !!varg; },
        [](const std::function<void(const Widget *,         int, int)> &varg) -> bool { return !!varg; },
        [](const std::function<void(const Widget *, void *, int, int)> &varg) -> bool { return !!varg; },

        [](std::nullptr_t){ return false; },
    },

    varDrawFunc);
}

void Widget::execDrawFunc(const Widget::VarDrawFunc &varDrawFunc, const Widget *widget, int argX, int argY)
{
    Widget::execDrawFunc(varDrawFunc, widget, nullptr, argX, argY);
}

void Widget::execDrawFunc(const Widget::VarDrawFunc &varDrawFunc, const Widget *widget, void *argPtr, int argX, int argY)
{
    std::visit(VarDispatcher
    {
        [                argX, argY](const std::function<void(                        int, int)> &varg) { if(varg){ varg(                argX, argY); }},
        [widget,         argX, argY](const std::function<void(const Widget *,         int, int)> &varg) { if(varg){ varg(widget,         argX, argY); }},
        [widget, argPtr, argX, argY](const std::function<void(const Widget *, void *, int, int)> &varg) { if(varg){ varg(widget, argPtr, argX, argY); }},

        [](std::nullptr_t){},
    },

    varDrawFunc);
}

Widget::Widget(Widget::InitArgs args)
    : WidgetTreeNode(args.parent.widget, args.parent.autoDelete)
    , m_dir(std::move(args.dir))
    , m_x  (std::make_pair(std::move(args.x), 0))
    , m_y  (std::make_pair(std::move(args.y), 0))
    , m_w  (std::move(args.w))
    , m_h  (std::move(args.h))
{
    for(auto &[childPtr, offDir, offX, offY, autoDelete]: args.childList){
        if(childPtr){
            addChildAt(childPtr, std::move(offDir), std::move(offX), std::move(offY), autoDelete);
        }
    }
}

void Widget::drawChild(const Widget *child, Widget::ROIMap m) const
{
    fflassert(child);
    fflassert(hasChild(child->id()));
    drawAsChild(child, DIR_UPLEFT, child->dx(), child->dy(), m);
}

void Widget::drawAsChild(const Widget *gfxWidget, dir8_t gfxDir, int gfxDx, int gfxDy, Widget::ROIMap m) const
{
    if(!gfxWidget){
        return;
    }

    if(!m.calibrate(this)){
        return;
    }

    gfxWidget->draw(m.create(Widget::ROI
    {
        .x = gfxDx - xSizeOff(gfxDir, gfxWidget->w()),
        .y = gfxDy - ySizeOff(gfxDir, gfxWidget->h()),

        .w = gfxWidget->w(),
        .h = gfxWidget->h(),
    }));
}

void Widget::drawRoot(Widget::ROIMap m) const
{
    fflassert(!parent());

    m.x += dx();
    m.y += dy();

    draw(m);
}

int Widget::sizeOff(int size, int index)
{
    /**/ if(index <  0) return        0;
    else if(index == 0) return size / 2;
    else                return size - 1;
}

int Widget::xSizeOff(dir8_t argDir, int argW)
{
    switch(argDir){
        case DIR_UPLEFT   : return sizeOff(argW, -1);
        case DIR_UP       : return sizeOff(argW,  0);
        case DIR_UPRIGHT  : return sizeOff(argW,  1);
        case DIR_RIGHT    : return sizeOff(argW,  1);
        case DIR_DOWNRIGHT: return sizeOff(argW,  1);
        case DIR_DOWN     : return sizeOff(argW,  0);
        case DIR_DOWNLEFT : return sizeOff(argW, -1);
        case DIR_LEFT     : return sizeOff(argW, -1);
        default           : return sizeOff(argW,  0);
    }
}

int Widget::ySizeOff(dir8_t argDir, int argH)
{
    switch(argDir){
        case DIR_UPLEFT   : return sizeOff(argH, -1);
        case DIR_UP       : return sizeOff(argH, -1);
        case DIR_UPRIGHT  : return sizeOff(argH, -1);
        case DIR_RIGHT    : return sizeOff(argH,  0);
        case DIR_DOWNRIGHT: return sizeOff(argH,  1);
        case DIR_DOWN     : return sizeOff(argH,  1);
        case DIR_DOWNLEFT : return sizeOff(argH,  1);
        case DIR_LEFT     : return sizeOff(argH,  0);
        default           : return sizeOff(argH,  0);
    }
}

void Widget::update(double fUpdateTime)
{
    foreachChild(false, [fUpdateTime, this](Widget *widget, bool)
    {
        widget->update(fUpdateTime);
    });
}

Widget *Widget::setProcessEvent(std::function<bool(Widget *, const SDL_Event &, bool, Widget::ROIMap)> argHandler)
{
    m_processEventHandler = std::move(argHandler);
    return this;
}

bool Widget::processEvent(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(m_processEventHandler){ return m_processEventHandler(this, event, valid, m); }
    else                     { return   processEventDefault(      event, valid, m); }
}

bool Widget::processEventRoot(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    fflassert(!parent());

    m.x += dx();
    m.y += dy();

    return processEvent(event, valid, m);
}

bool Widget::processEventParent(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    const auto par = parent();
    fflassert(par);

    if(!m.calibrate(par)){
        return false;
    }

    if(!mathf::cropChildROI(
                std::addressof(m.ro->x), std::addressof(m.ro->y),
                std::addressof(m.ro->w), std::addressof(m.ro->h),

                std::addressof(m.x),
                std::addressof(m.y),

                par->w(),
                par->h(),

                dx(),
                dy(),
                w(),
                h())){
        return false;
    }
    return processEvent(event, valid, m);
}

bool Widget::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    bool took = false;
    uint64_t focusedWidgetID = 0;

    foreachChild(false, [&event, valid, &took, &focusedWidgetID, &m, this](Widget *widget, bool)
    {
        if(widget->show()){
            const bool validEvent = valid && !took;
            const bool takenEvent = widget->processEventParent(event, validEvent, m);

            if(!validEvent && takenEvent){
                throw fflerror("widget %s takes invalid event", widget->name());
            }

            // it's possible that a widget takes event but doesn't get focus
            // i.e. press a button to pop up a modal window, but still abort here for easier maintenance

            if(widget->focus()){
                if(focusedWidgetID){
                    if(auto focusedWidget = hasChild(focusedWidgetID); focusedWidget && focusedWidget->focus()){
                        // a widget with focus can drop events
                        // i.e. a focused slider ignores mouse motion if button released
                        focusedWidget->setFocus(false);
                    }
                }
                focusedWidgetID = widget->id();
            }

            took |= takenEvent;
        }
    });

    if(auto widget = hasChild(focusedWidgetID)){
        moveBack(widget);
    }

    return took;
}

void Widget::draw(Widget::ROIMap m) const
{
    foreachChild([&m, this](const Widget *widget, bool)
    {
        if(widget->show()){
            drawChild(widget, m);
        }
    });
}

Widget *Widget::setAfterResize(std::function<void(Widget *)> argHandler)
{
    m_afterResizeHandler = std::move(argHandler);
    return this;
}

void Widget::afterResize()
{
    if(m_afterResizeHandler){
        m_afterResizeHandler(this);
    }
    else{
        afterResizeDefault();
    }
}

void Widget::afterResizeDefault()
{
    foreachChild([](Widget *child, bool){ child->afterResize(); });
}

int Widget::w() const
{
    const RecursionDetector hDetect(m_wCalc, name(), "w()");
    return Widget::evalSizeOpt(m_w, this, [this]()
    {
        int maxW = 0;
        foreachChild([&maxW](const Widget *widget, bool)
        {
            if(widget->localShow()){
                maxW = std::max<int>(maxW, widget->dx() + widget->w());
            }
        });
        return maxW;
    });
}

int Widget::h() const
{
    const RecursionDetector hDetect(m_hCalc, name(), "h()");
    return Widget::evalSizeOpt(m_h, this, [this]()
    {
        int maxH = 0;
        foreachChild([&maxH](const Widget *widget, bool)
        {
            if(widget->localShow()){
                maxH = std::max<int>(maxH, widget->dy() + widget->h());
            }
        });
        return maxH;
    });
}

int Widget::dx() const
{
    return Widget::evalInt(m_x.first, this) + m_x.second - xSizeOff(dir(), w());
}

int Widget::dy() const
{
    return Widget::evalInt(m_y.first, this) + m_y.second - ySizeOff(dir(), h());
}

static int _rd_helper(const Widget *a, const Widget *b, const auto func)
{
    const auto fnTraverse = [&func](const Widget *w) -> std::tuple<const Widget *, int>
    {
        const Widget *root= nullptr;
        int off = 0;

        while(w){
            off += func(w);
            if(const auto par = w->parent()){
                w = par;
            }
            else{
                root = w;
                break;
            }
        }

        return {root, off};
    };

    const auto [ra, offa] = fnTraverse(a);
    const auto [rb, offb] = fnTraverse(b);

    if(ra == rb){
        return offa - offb;
    }

    throw fflerror("widgets from different trees: %p vs %p", to_cvptr(a), to_cvptr(b));
}


int Widget::rdx(const Widget* other) const
{
    if(!other || (other == this)){
        return 0;
    }

    return _rd_helper(this, other, [](const Widget *w) { return w->dx(); });
}

int Widget::rdy(const Widget* other) const
{
    if(!other || (other == this)){
        return 0;
    }

    return _rd_helper(this, other, [](const Widget *w) { return w->dy(); });
}

Widget *Widget::setData(std::any argData)
{
    m_data = std::move(argData);
    return this;
}

bool Widget::focus() const
{
    bool hasFocus = false;
    foreachChild([&hasFocus](const Widget * widget, bool) -> bool
    {
        return hasFocus = widget->focus();
    });

    if(hasFocus){
        return true;
    }

    return m_focus;
}

Widget *Widget::setFocus(bool argFocus)
{
    foreachChild([](Widget * widget, bool)
    {
        widget->setFocus(false);
    });

    m_focus = argFocus;
    return this;
}

// focus helper
// we have tons of code like:
//
//     if(...){
//         p->focus(true);  // get focus
//         return true;     // since the event changes focus, then event is consumed
//     }
//     else{
//         p->focus(false); // event doesn't help to move focus to the widget
//         return false;    // not consumed, try next widget
//     }
//
// this function helps to simplify the code to:
//
//     return p->consumeFocus(...)

bool Widget::consumeFocus(bool argFocus, Widget *child)
{
    if(argFocus){
        if(child){
            if(hasChild(child->id())){
                if(child->focus()){
                    // don't setup here
                    // when we setup focus in a deep call, this preserve focus of deep sub-widget
                }
                else{
                    setFocus(false);
                    child->setFocus(true);
                }
            }
            else{
                throw fflerror("widget has no child: %p", to_cvptr(child));
            }
        }
        else{
            setFocus(true);
        }
    }
    else{
        if(child){
            throw fflerror("unexpected child: %p", to_cvptr(child));
        }
        else{
            setFocus(false);
        }
    }
    return argFocus;
}

bool Widget::show() const
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

bool Widget::localShow() const
{
    return Widget::evalBool(m_show.first, this) != m_show.second;
}

void Widget::flipShow()
{
    m_show.second = !m_show.second;
}

Widget *Widget::setShow(Widget::VarBool argShow)
{
    m_show = std::make_pair(std::move(argShow), false);
    return this;
}

bool Widget::active() const
{
    if(m_parent && !m_parent->active()){
        return false;
    }
    return localActive();
}

bool Widget::localActive() const
{
    return Widget::evalBool(m_active.first, this) != m_active.second;
}

void Widget::flipActive()
{
    m_active.second = !m_active.second;
}

Widget *Widget::setActive(Widget::VarBool argActive)
{
    m_active = std::make_pair(std::move(argActive), false);
    return this;
}

void Widget::moveXTo(Widget::VarOff arg)
{
    m_x = std::make_pair(std::move(arg), 0);
}

void Widget::moveYTo(Widget::VarOff arg)
{
    m_y = std::make_pair(std::move(arg), 0);
}

void Widget::moveTo(Widget::VarOff argX, Widget::VarOff argY)
{
    moveXTo(std::move(argX));
    moveYTo(std::move(argY));
}

void Widget::moveBy(Widget::VarOff argDX, Widget::VarOff argDY)
{
    const auto fnOp = [](std::pair<Widget::VarOff, int> &offset, Widget::VarOff update)
    {
        if(update.index() == 0){
            offset.second += std::get<int>(update);
        }
        else if(offset.first.index() == 0){
            offset.second += std::get<int>(offset.first);
            offset.first   = std::move(update);
        }
        else{
            offset.first = [u = std::move(offset.first), v = std::move(update)](const Widget *widgetPtr)
            {
                return std::get<std::function<int(const Widget *)>>(u)(widgetPtr)
                     + std::get<std::function<int(const Widget *)>>(v)(widgetPtr);
            };
        }
    };

    fnOp(m_x, std::move(argDX));
    fnOp(m_y, std::move(argDY));
}

void Widget::moveBy(Widget::VarOff argDX, Widget::VarOff argDY, const Widget::ROI &r)
{
    moveBy(std::move(argDX), std::move(argDY));
    if(const auto t = dx(); r.x > t){
        m_x.second += (r.x - t);
    }

    if(const auto t = dx() + w(); t > r.x + r.w){
        m_x.second -= (t - (r.x + r.w));
    }

    if(const auto t = dy(); r.y > t){
        m_y.second += (r.y - t);
    }

    if(const auto t = dy() + h(); t > r.y + r.h){
        m_y.second -= (t - (r.y + r.h));
    }
}

void Widget::moveAt(Widget::VarDir argDir, Widget::VarOff argX, Widget::VarOff argY)
{
    m_dir = std::move(argDir);
    m_x   = std::make_pair(std::move(argX), 0);
    m_y   = std::make_pair(std::move(argY), 0);
}

Widget *Widget::disableSetSize()
{
    m_canSetSize = false; // can not flip back
    return this;
}

Widget *Widget::setW(Widget::VarSizeOpt argSize)
{
    if(m_canSetSize){
        m_w = std::move(argSize);
        return this;
    }
    else{
        throw fflerror("can not resize %s", name());
    }
}

Widget *Widget::setH(Widget::VarSizeOpt argSize)
{
    if(m_canSetSize){
        m_h = std::move(argSize);
        return this;
    }
    else{
        throw fflerror("can not resize %s", name());
    }
}

Widget *Widget::setSize(Widget::VarSizeOpt argW, Widget::VarSizeOpt argH)
{
    return setW(std::move(argW))->setH(std::move(argH));
}
