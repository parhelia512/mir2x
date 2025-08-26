#include <atomic>
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

bool Widget::hasIntDir(const Widget::VarDir &varDir)
{
    return varDir.index() == 0;
}

bool Widget::hasFuncDir(const Widget::VarDir &varDir)
{
    return varDir.index() == 1;
}

dir8_t Widget::asIntDir(const Widget::VarDir &varDir)
{
    return std::get<dir8_t>(varDir);
}

dir8_t &Widget::asIntDir(Widget::VarDir &varDir)
{
    return std::get<dir8_t>(varDir);
}

const std::function<dir8_t(const Widget *)> &Widget::asFuncDir(const Widget::VarDir &varDir)
{
    return std::get<std::function<dir8_t(const Widget *)>>(varDir);
}

std::function<dir8_t(const Widget *)> &Widget::asFuncDir(Widget::VarDir &varDir)
{
    return std::get<std::function<dir8_t(const Widget *)>>(varDir);
}

dir8_t Widget::evalDir(const Widget::VarDir &varDir, const Widget *widgetPtr)
{
    return std::visit(VarDispatcher
    {
        [](const dir8_t &arg)
        {
            return arg;
        },

        [widgetPtr](const auto &arg)
        {
            return arg ? arg(widgetPtr) : throw fflerror("invalid argument");
        },
    }, varDir);
}

bool Widget::hasIntOff(const Widget::VarOff &varOff)
{
    return varOff.index() == 0;
}

bool Widget::hasFuncOff(const Widget::VarOff &varOff)
{
    return varOff.index() == 1;
}

int Widget::asIntOff(const Widget::VarOff &varOff)
{
    return std::get<int>(varOff);
}

int &Widget::asIntOff(Widget::VarOff &varOff)
{
    return std::get<int>(varOff);
}

const std::function<int(const Widget *)> &Widget::asFuncOff(const Widget::VarOff &varOff)
{
    return std::get<std::function<int(const Widget *)>>(varOff);
}

std::function<int(const Widget *)> &Widget::asFuncOff(Widget::VarOff &varOff)
{
    return std::get<std::function<int(const Widget *)>>(varOff);
}

int Widget::evalOff(const Widget::VarOff &varOffset, const Widget *widgetPtr)
{
    return std::visit(VarDispatcher
    {
        [](const int &arg)
        {
            return arg;
        },

        [widgetPtr](const auto &arg)
        {
            return arg ? arg(widgetPtr) : throw fflerror("invalid argument");
        },
    }, varOffset);
}

bool Widget::hasSize(const Widget::VarSize &varSize)
{
    return varSize.index() != 0;
}

bool Widget::hasIntSize(const Widget::VarSize &varSize)
{
    return varSize.index() == 1;
}

bool Widget::hasFuncSize(const Widget::VarSize &varSize)
{
    return varSize.index() == 2;
}

int Widget::asIntSize(const Widget::VarSize &varSize)
{
    return std::get<int>(varSize);
}

int &Widget::asIntSize(Widget::VarSize &varSize)
{
    return std::get<int>(varSize);
}

const std::function<int(const Widget *)> &Widget::asFuncSize(const Widget::VarSize &varSize)
{
    return std::get<std::function<int(const Widget *)>>(varSize);
}

std::function<int(const Widget *)> &Widget::asFuncSize(Widget::VarSize &varSize)
{
    return std::get<std::function<int(const Widget *)>>(varSize);
}

bool Widget::hasBoolFlag(const Widget::VarFlag &varFlag)
{
    return varFlag.index() == 0;
}

bool Widget::hasFuncFlag(const Widget::VarFlag &varFlag)
{
    return varFlag.index() == 1;
}

bool Widget::asBoolFlag(const Widget::VarFlag &varFlag)
{
    return std::get<bool>(varFlag);
}

bool &Widget::asBoolFlag(Widget::VarFlag &varFlag)
{
    return std::get<bool>(varFlag);
}

const std::function<bool(const Widget *)> &Widget::asFuncFlag(const Widget::VarFlag &varFlag)
{
    return std::get<std::function<bool(const Widget *)>>(varFlag);
}

std::function<bool(const Widget *)> &Widget::asFuncFlag(Widget::VarFlag &varFlag)
{
    return std::get<std::function<bool(const Widget *)>>(varFlag);
}

bool Widget::evalFlag(const Widget::VarFlag &varFlag, const Widget *widgetPtr)
{
    return std::visit(VarDispatcher
    {
        [](const bool &arg)
        {
            return arg;
        },

        [widgetPtr](const auto &arg)
        {
            return arg ? arg(widgetPtr) : throw fflerror("invalid argument");
        },
    }, varFlag);
}

bool Widget::hasU32Color(const Widget::VarColor &varColor)
{
    return varColor.index() == 0;
}

bool Widget::hasFuncColor(const Widget::VarColor &varColor)
{
    return varColor.index() == 1;
}

uint32_t Widget::asU32Color(const Widget::VarColor &varColor)
{
    return std::get<uint32_t>(varColor);
}

uint32_t &Widget::asU32Color(Widget::VarColor &varColor)
{
    return std::get<uint32_t>(varColor);
}

const std::function<uint32_t(const Widget *)> &Widget::asFuncColor(const Widget::VarColor &varColor)
{
    return std::get<std::function<uint32_t(const Widget *)>>(varColor);
}

std::function<uint32_t(const Widget *)> &Widget::asFuncColor(Widget::VarColor &varColor)
{
    return std::get<std::function<uint32_t(const Widget *)>>(varColor);
}

uint32_t Widget::evalColor(const Widget::VarColor &varColor, const Widget *widgetPtr)
{
    return std::visit(VarDispatcher
    {
        [](const uint32_t &arg)
        {
            return arg;
        },

        [widgetPtr](const auto &arg)
        {
            return arg ? arg(widgetPtr) : throw fflerror("invalid argument");
        },
    }, varColor);
}

Widget::Widget(
        Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        Widget::VarSize argW,
        Widget::VarSize argH,

        std::vector<std::tuple<Widget *, Widget::VarDir, Widget::VarOff, Widget::VarOff, bool>> argChildList,

        Widget *argParent,
        bool    argAutoDelete)

    : WidgetTreeNode(argParent, argAutoDelete)
    , m_dir(std::move(argDir))
    , m_x  (std::make_pair(std::move(argX), 0))
    , m_y  (std::make_pair(std::move(argY), 0))
    , m_w  (std::move(argW))
    , m_h  (std::move(argH))
{
    // don't check if w/h is a function
    // because it may refers to sub-widget which has not be initialized yet

    if(Widget::hasFuncDir (m_dir      )){ fflassert(Widget::asFuncDir (m_dir      ), m_dir      ); }
    if(Widget::hasFuncOff (m_x.first  )){ fflassert(Widget::asFuncOff (m_x.first  ), m_x.first  ); }
    if(Widget::hasFuncOff (m_y.first  )){ fflassert(Widget::asFuncOff (m_y.first  ), m_y.first  ); }
    if(Widget::hasFuncSize(m_w        )){ fflassert(Widget::asFuncSize(m_w        ), m_w        ); }
    if(Widget::hasFuncSize(m_h        )){ fflassert(Widget::asFuncSize(m_h        ), m_h        ); }

    if(Widget::hasIntSize(m_w)){ fflassert(Widget::asIntSize(m_w) >= 0, m_w); }
    if(Widget::hasIntSize(m_h)){ fflassert(Widget::asIntSize(m_h) >= 0, m_h); }

    for(auto &[childPtr, offDir, offX, offY, autoDelete]: argChildList){
        if(childPtr){
            addChildAt(childPtr, std::move(offDir), std::move(offX), std::move(offY), autoDelete);
        }
    }
}

void Widget::drawChildEx(
        const Widget *child,

        int dstX,
        int dstY,

        int srcX,
        int srcY,

        int srcW,
        int srcH) const
{
    fflassert(child);
    fflassert(hasChild(child->id()));

    if(!child->show()){
        return;
    }

    int drawSrcX = srcX;
    int drawSrcY = srcY;
    int drawSrcW = srcW;
    int drawSrcH = srcH;
    int drawDstX = dstX;
    int drawDstY = dstY;

    if(mathf::cropChildROI(
                &drawSrcX, &drawSrcY,
                &drawSrcW, &drawSrcH,
                &drawDstX, &drawDstY,

                w(),
                h(),

                child->dx(),
                child->dy(),
                child-> w(),
                child-> h())){
        child->drawEx(drawDstX, drawDstY, drawSrcX, drawSrcY, drawSrcW, drawSrcH);
    }
}

void Widget::drawAt(
        dir8_t dstDir,
        int    dstX,
        int    dstY,

        int dstRgnX = 0,
        int dstRgnY = 0,

        int dstRgnW = -1,
        int dstRgnH = -1) const
{
    int srcXCrop = 0;
    int srcYCrop = 0;
    int dstXCrop = dstX - xSizeOff(dstDir, w());
    int dstYCrop = dstY - ySizeOff(dstDir, h());
    int srcWCrop = w();
    int srcHCrop = h();

    if(mathf::cropROI(
                &srcXCrop, &srcYCrop,
                &srcWCrop, &srcHCrop,
                &dstXCrop, &dstYCrop,

                w(),
                h(),

                0, 0, -1, -1,
                dstRgnX, dstRgnY, dstRgnW, dstRgnH)){
        drawEx(dstXCrop, dstYCrop, srcXCrop, srcYCrop, srcWCrop, srcHCrop);
    }
}

void Widget::drawRoot()
{
    fflassert(!parent());
    drawEx(dx(), dy(), 0, 0, w(), h());
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

Widget *Widget::setProcessEvent(std::function<bool(Widget *, const SDL_Event &, bool, int, int)> argHandler)
{
    m_processEventHandler = std::move(argHandler);
    return this;
}

bool Widget::processEvent(const SDL_Event &event, bool valid, int startDstX, int startDstY)
{
    if(m_processEventHandler){ return m_processEventHandler(this, event, valid, startDstX, startDstY); }
    else                     { return   processEventDefault(      event, valid, startDstX, startDstY); }
}

bool Widget::processParentEvent(const SDL_Event &event, bool valid, int startDstX, int startDstY)
{
    return processEvent(event, valid, startDstX + dx(), startDstY + dy());
}

bool Widget::applyRootEvent(const SDL_Event &event)
{
    fflassert(!parent());
    return processEvent(event, true, dx(), dy());
}

bool Widget::processEventDefault(const SDL_Event &event, bool valid, int startDstX, int startDstY)
{
    if(!show()){
        return false;
    }

    bool took = false;
    uint64_t focusedWidgetID = 0;

    foreachChild(false, [&event, valid, &took, &focusedWidgetID, startDstX, startDstY, this](Widget *widget, bool)
    {
        if(widget->show()){
            const bool validEvent = valid && !took;
            const bool takenEvent = widget->processParentEvent(event, validEvent, startDstX, startDstY);

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

void Widget::drawEx(int dstX, int dstY, int srcX, int srcY, int srcW, int srcH) const
{
    if(!show()){
        return;
    }

    foreachChild([srcX, srcY, dstX, dstY, srcW, srcH, this](const Widget *widget, bool)
    {
        if(widget->show()){
            int srcXCrop = srcX;
            int srcYCrop = srcY;
            int dstXCrop = dstX;
            int dstYCrop = dstY;
            int srcWCrop = srcW;
            int srcHCrop = srcH;

            if(mathf::cropChildROI(
                        &srcXCrop, &srcYCrop,
                        &srcWCrop, &srcHCrop,
                        &dstXCrop, &dstYCrop,

                        w(),
                        h(),

                        widget->dx(),
                        widget->dy(),
                        widget-> w(),
                        widget-> h())){
                widget->drawEx(dstXCrop, dstYCrop, srcXCrop, srcYCrop, srcWCrop, srcHCrop);
            }
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
    const auto width = std::visit(VarDispatcher
    {
        [](const int &arg)
        {
            return arg;
        },

        [this](const std::function<int(const Widget *)> &arg)
        {
            return arg ? arg(this) : throw fflerror("invalid argument");
        },

        [this](const auto &) // auto-scaling mode
        {
            int maxW = 0;
            foreachChild([&maxW](const Widget *widget, bool)
            {
                if(widget->localShow()){
                    maxW = std::max<int>(maxW, widget->dx() + widget->w());
                }
            });
            return maxW;
        }
    }, m_w);

    fflassert(width >= 0, width, m_w);
    return width;
}

int Widget::h() const
{
    const RecursionDetector hDetect(m_hCalc, name(), "h()");
    const auto height = std::visit(VarDispatcher
    {
        [](const int &arg)
        {
            return arg;
        },

        [this](const std::function<int(const Widget *)> &arg)
        {
            return arg ? arg(this) : throw fflerror("invalid argument");
        },

        [this](const auto &) // auto-scaling mode
        {
            int maxH = 0;
            foreachChild([&maxH](const Widget *widget, bool)
            {
                if(widget->localShow()){
                    maxH = std::max<int>(maxH, widget->dy() + widget->h());
                }
            });

            return maxH;
        }
    }, m_h);

    fflassert(height >= 0, m_h);
    return height;
}

bool Widget::in(int pixelX, int pixelY, int startDstX, int startDstY) const
{
    return mathf::pointInRectangle<int>(pixelX, pixelY, startDstX, startDstY, w(), h());
}

bool Widget::parentIn(int pixelX, int pixelY, int startDstX, int startDstY) const
{
    return in(pixelX, pixelY, startDstX + dx(), startDstY + dy());
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

Widget *Widget::setW(Widget::VarSize argSize)
{
    if(m_disableSetSize){
        throw fflerror("can not resize %s", name());
    }

    m_w = std::move(argSize);
    return this;
}

virtual Widget *setH(Widget::VarSize argSize) final
{
    if(m_disableSetSize){
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
