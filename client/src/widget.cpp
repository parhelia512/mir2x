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

void WidgetTreeNode::addChildAt(Widget *argWidget, WidgetTreeNode::VarDir argDir, WidgetTreeNode::VarInt argX, WidgetTreeNode::VarInt argY, bool argAutoDelete)
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

bool Widget::ROI::crop(Widget::ROI &r) const
{
    if(r.empty()){
        return false;
    }

    // return mathf::cropSegment<int>(r.x, r.w, this->x, this->w)
    //     && mathf::cropSegment<int>(r.y, r.h, this->y, this->h);

    return mathf::cropSegment(r.x, r.w, this->x, this->w)
        && mathf::cropSegment(r.y, r.h, this->y, this->h);
}

bool Widget::ROI::overlap(const Widget::ROI &rhs) const
{
    return mathf::rectangleOverlap(x, y, w, h, rhs.x, rhs.y, rhs.w, rhs.h);
}

Widget::ROIOpt::ROIOpt(int argX, int argY, int argW, int argH)
    : m_roiOpt(Widget::ROI{argX, argY, argW, argH})
{}

Widget::ROIOpt::ROIOpt(const Widget::ROI &roi)
    : m_roiOpt(roi)
{}

Widget::ROI Widget::ROIOpt::evalROI(const Widget *widget) const
{
    fflassert(widget);
    return evalROI(widget->roi());
}

Widget::ROI Widget::ROIOpt::evalROI(const Widget::ROI &roi) const
{
    if(m_roiOpt.has_value()){
        auto currROI = m_roiOpt.value();
        roi.crop(currROI);
        return currROI;
    }
    else{
        return roi;
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

int Widget::evalInt(const Widget::VarInt &varOffset, const Widget *widget, const void *arg)
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

Widget::Widget(
        Widget::VarDir argDir,
        Widget::VarInt argX,
        Widget::VarInt argY,

        Widget::VarSizeOpt argW,
        Widget::VarSizeOpt argH,

        std::vector<std::tuple<Widget *, Widget::VarDir, Widget::VarInt, Widget::VarInt, bool>> argChildList,

        Widget *argParent,
        bool    argAutoDelete)

    : WidgetTreeNode(argParent, argAutoDelete)
    , m_dir(std::move(argDir))
    , m_x  (std::make_pair(std::move(argX), 0))
    , m_y  (std::make_pair(std::move(argY), 0))
    , m_w  (std::move(argW))
    , m_h  (std::move(argH))
{
    for(auto &[childPtr, offDir, offX, offY, autoDelete]: argChildList){
        if(childPtr){
            addChildAt(childPtr, std::move(offDir), std::move(offX), std::move(offY), autoDelete);
        }
    }
}

void Widget::drawChildEx(const Widget *child, int dstX, int dstY, const Widget::ROIOpt &roi) const
{
    fflassert(child);
    fflassert(hasChild(child->id()));
    drawAsChildEx(child, DIR_UPLEFT, child->dx(), child->dy(), dstX, dstY, roi);
}

void Widget::drawAt(dir8_t dstDir, int dstX, int dstY, const Widget::ROIOpt &roi) const
{
    auto roiOpt = cropDrawROI(dstX, dstY, roi);
    if(!roiOpt.has_value()){
        return;
    }

    dstX -= xSizeOff(dstDir, w());
    dstY -= ySizeOff(dstDir, h());

    if(!mathf::cropROI(
                &roiOpt->x, &roiOpt->y,
                &roiOpt->w, &roiOpt->h,

                &dstX,
                &dstY,

                w(),
                h(),

                roiOpt->x, roiOpt->y,
                roiOpt->w, roiOpt->h)){
        return;
    }

    drawEx(dstX, dstY, roiOpt.value());
}

void Widget::drawAsChildEx(
        const Widget *gfxWidget,

        dir8_t gfxDir,
        int    gfxDx,
        int    gfxDy,

        int dstX,
        int dstY,
        const Widget::ROIOpt &roi) const
{
    auto roiOpt = cropDrawROI(dstX, dstY, roi);
    if(!roiOpt.has_value()){
        return;
    }

    if(!gfxWidget){
        return;
    }

    if(!mathf::cropChildROI(
                &roiOpt->x, &roiOpt->y,
                &roiOpt->w, &roiOpt->h,

                &dstX,
                &dstY,

                w(),
                h(),

                gfxDx - xSizeOff(gfxDir, gfxWidget->w()),
                gfxDy - ySizeOff(gfxDir, gfxWidget->h()),

                gfxWidget->w(),
                gfxWidget->h())){
        return;
    }

    gfxWidget->drawEx(dstX, dstY, roiOpt.value());
}

void Widget::drawRoot(int rootDstX, int rootDstY) const
{
    fflassert(!parent());
    drawEx(dx() + rootDstX, dy() + rootDstY, std::nullopt);
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

Widget *Widget::setProcessEvent(std::function<bool(Widget *, const SDL_Event &, bool, int, int, const Widget::ROIOpt &)> argHandler)
{
    m_processEventHandler = std::move(argHandler);
    return this;
}

bool Widget::processEvent(const SDL_Event &event, bool valid, int startDstX, int startDstY, const Widget::ROIOpt &roi)
{
    if(m_processEventHandler){ return m_processEventHandler(this, event, valid, startDstX, startDstY, roi); }
    else                     { return   processEventDefault(      event, valid, startDstX, startDstY, roi); }
}

// {startDstX, startDstY, roi} is based on parent widget
// calculate sub-roi for current child widget
bool Widget::processParentEvent(const SDL_Event &event, bool valid, int startDstX, int startDstY, const Widget::ROIOpt &roi)
{
    const auto par = parent();
    fflassert(par);

    auto roiOpt = par->cropDrawROI(startDstX, startDstY, roi);
    if(!roiOpt.has_value()){
        return false;
    }

    if(!mathf::cropChildROI(
                &roiOpt->x, &roiOpt->y,
                &roiOpt->w, &roiOpt->h,

                &startDstX,
                &startDstY,

                par->w(),
                par->h(),

                dx(),
                dy(),
                w(),
                h())){
        return false;
    }
    return processEvent(event, valid, startDstX, startDstY, roiOpt.value());
}

bool Widget::processRootEvent(const SDL_Event &event, bool valid, int rootDstX, int rootDstY)
{
    fflassert(!parent());
    return processEvent(event, valid, dx() + rootDstX, dy() + rootDstY, std::nullopt);
}

bool Widget::processEventDefault(const SDL_Event &event, bool valid, int startDstX, int startDstY, const Widget::ROIOpt &roi)
{
    bool took = false;
    uint64_t focusedWidgetID = 0;

    foreachChild(false, [&event, valid, &took, &focusedWidgetID, startDstX, startDstY, &roi, this](Widget *widget, bool)
    {
        if(widget->show()){
            const bool validEvent = valid && !took;
            const bool takenEvent = widget->processParentEvent(event, validEvent, startDstX, startDstY, roi);

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

void Widget::drawEx(int dstX, int dstY, const Widget::ROIOpt &roi) const
{
    foreachChild([dstX, dstY, &roi, this](const Widget *widget, bool)
    {
        if(widget->show()){
            drawChildEx(widget, dstX, dstY, roi);
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

int Widget::rdx(const Widget *widget) const
{
    if(widget){
        if(const auto parptr = parent()){
            if(widget == parptr){
                return dx();
            }
            else{
                return dx() + parptr->rdx(widget);
            }
        }
        else{
            throw fflerror("invalid parent");
        }
    }
    else{
        return dx();
    }
}

int Widget::rdy(const Widget *widget) const
{
    if(widget){
        if(const auto par = parent()){
            if(widget == par){
                return dy();
            }
            else{
                return dy() + par->rdy(widget);
            }
        }
        else{
            throw fflerror("invalid parent");
        }
    }
    else{
        return dy();
    }
}

std::optional<Widget::ROI> Widget::cropDrawROI(int &dstX, int &dstY, const Widget::ROIOpt &roi) const
{
    const auto srcROI = roi.evalROI(this);
    if(srcROI.empty()){
        return std::nullopt;
    }

    const auto srcXDiff = srcROI.x - roi.get([](const auto &r){ return r.x; }, 0);
    const auto srcYDiff = srcROI.y - roi.get([](const auto &r){ return r.y; }, 0);

    dstX += srcXDiff;
    dstY += srcYDiff;

    return srcROI;
}

Widget *Widget::setData(std::any argData)
{
    m_data = std::move(argData);
    return this;
}

bool Widget::in(int pixelX, int pixelY, int startDstX, int startDstY, const Widget::ROIOpt &roi) const
{
    if(const auto roiOpt = cropDrawROI(startDstX, startDstY, roi); roiOpt.has_value()){
        return roiOpt.value().in(pixelX - startDstX, pixelY - startDstY);
    }
    return false;
}

bool Widget::parentIn(int pixelX, int pixelY, int startDstX, int startDstY, const Widget::ROIOpt &roi) const
{
    // helper function used in parents' drawEx()/processEventDefault()
    // check if {pixelX, pixelY} is in current child, when its parent only viewed by: {startDstX, startDstY, roi}

    const auto par = parent();
    fflassert(par);

    auto roiOpt = par->cropDrawROI(startDstX, startDstY, roi);
    if(!roiOpt.has_value()){
        return false;
    }

    if(!mathf::cropChildROI(
                &roiOpt->x, &roiOpt->y,
                &roiOpt->w, &roiOpt->h,

                &startDstX,
                &startDstY,

                par->w(),
                par->h(),

                dx(),
                dy(),
                w(),
                h())){
        return false;
    }

    return in(pixelX, pixelY, startDstX, startDstY, roiOpt.value());
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

void Widget::moveXTo(Widget::VarInt arg)
{
    m_x = std::make_pair(std::move(arg), 0);
}

void Widget::moveYTo(Widget::VarInt arg)
{
    m_y = std::make_pair(std::move(arg), 0);
}

void Widget::moveTo(Widget::VarInt argX, Widget::VarInt argY)
{
    moveXTo(std::move(argX));
    moveYTo(std::move(argY));
}

void Widget::moveBy(Widget::VarInt dx, Widget::VarInt dy)
{
    const auto fnOp = [](std::pair<Widget::VarInt, int> &offset, Widget::VarInt update)
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

    fnOp(m_x, std::move(dx));
    fnOp(m_y, std::move(dy));
}

void Widget::moveAt(Widget::VarDir argDir, Widget::VarInt argX, Widget::VarInt argY)
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
