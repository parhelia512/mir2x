#include <atomic>
#include "mathf.hpp"
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

bool Widget::hasU32(const Widget::VarU32 &varU32)
{
    return varU32.index() == 0;
}

bool Widget::hasFuncU32(const Widget::VarU32 &varU32)
{
    return varU32.index() == 1;
}

uint32_t Widget::asU32(const Widget::VarU32 &varU32)
{
    return std::get<uint32_t>(varU32);
}

uint32_t &Widget::asU32(Widget::VarU32 &varU32)
{
    return std::get<uint32_t>(varU32);
}

const std::function<uint32_t(const Widget *)> &Widget::asFuncU32(const Widget::VarU32 &varU32)
{
    return std::get<std::function<uint32_t(const Widget *)>>(varU32);
}

std::function<uint32_t(const Widget *)> &Widget::asFuncU32(Widget::VarU32 &varU32)
{
    return std::get<std::function<uint32_t(const Widget *)>>(varU32);
}

uint32_t Widget::evalU32(const Widget::VarU32 &varU32, const Widget *widgetPtr)
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
    }, varU32);
}

bool Widget::hasBlendMode(const Widget::VarBlendMode &varBlendMode)
{
    return varBlendMode.index() == 0;
}

bool Widget::hasFuncBlendMode(const Widget::VarBlendMode &varBlendMode)
{
    return varBlendMode.index() == 1;
}

SDL_BlendMode Widget::asBlendMode(const Widget::VarBlendMode &varBlendMode)
{
    return std::get<SDL_BlendMode>(varBlendMode);
}

SDL_BlendMode &Widget::asBlendMode(Widget::VarBlendMode &varBlendMode)
{
    return std::get<SDL_BlendMode>(varBlendMode);
}

const std::function<SDL_BlendMode(const Widget *)> &Widget::asFuncBlendMode(const Widget::VarBlendMode &varBlendMode)
{
    return std::get<std::function<SDL_BlendMode(const Widget *)>>(varBlendMode);
}

std::function<SDL_BlendMode(const Widget *)> &Widget::asFuncBlendMode(Widget::VarBlendMode &varBlendMode)
{
    return std::get<std::function<SDL_BlendMode(const Widget *)>>(varBlendMode);
}

SDL_BlendMode Widget::evalBlendMode(const Widget::VarBlendMode &varBlendMode, const Widget *widget)
{
    return std::visit(VarDispatcher
    {
        [](SDL_BlendMode arg)
        {
            return arg;
        },

        [widget](const auto &arg)
        {
            return arg ? arg(widget) : throw fflerror("invalid argument");
        },
    }, varBlendMode);
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

void Widget::drawChildEx(const Widget *child, int dstX, int dstY, const Widget::ROIOpt &roi) const
{
    fflassert(child);
    fflassert(hasChild(child->id()));
    drawAsChildEx(child, child->dir(), child->dx(), child->dy(), dstX, dstY, roi);
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
bool Widget::processParentEvent(const SDL_Event &event, bool valid, int parentW, int parentH, int startDstX, int startDstY, const Widget::ROIOpt &roi)
{
    auto roiOpt = cropDrawROI(startDstX, startDstY, roi);
    if(!roiOpt.has_value()){
        return false;
    }

    if(!mathf::cropChildROI(
                &roiOpt->x, &roiOpt->y,
                &roiOpt->w, &roiOpt->h,

                &startDstX,
                &startDstY,

                parentW,
                parentH,

                dx(),
                dy(),
                w(),
                h())){
        return false;
    }
    return processEvent(event, valid, startDstX, startDstY, roiOpt.value());
}

bool Widget::applyRootEvent(const SDL_Event &event, bool valid, int rootDstX, int rootDstY)
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
            const bool takenEvent = widget->processParentEvent(event, validEvent, w(), h(), startDstX, startDstY, roi);

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

int Widget::dx() const
{
    return Widget::evalOff(m_x.first, this) + m_x.second - xSizeOff(dir(), w());
}

int Widget::dy() const
{
    return Widget::evalOff(m_y.first, this) + m_y.second - ySizeOff(dir(), h());
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

bool Widget::parentIn(int pixelX, int pixelY, int parentW, int parentH, int startDstX, int startDstY, const Widget::ROIOpt &roi) const
{
    // helper function used in parents' drawEx()/processEventDefault()
    // check if {pixelX, pixelY} is in current child, when its parent only viewed by: {startDstX, startDstY, roi}

    auto roiOpt = cropDrawROI(startDstX, startDstY, roi);
    if(!roiOpt.has_value()){
        return false;
    }

    if(!mathf::cropChildROI(
                &roiOpt->x, &roiOpt->y,
                &roiOpt->w, &roiOpt->h,

                &startDstX,
                &startDstY,

                parentW,
                parentH,

                dx(),
                dy(),
                w (),
                h ())){
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
    return Widget::evalFlag(m_show.first, this) != m_show.second;
}

void Widget::flipShow()
{
    m_show.second = !m_show.second;
}

Widget *Widget::setShow(Widget::VarFlag argShow)
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
    return Widget::evalFlag(m_active.first, this) != m_active.second;
}

void Widget::flipActive()
{
    m_active.second = !m_active.second;
}

Widget *Widget::setActive(Widget::VarFlag argActive)
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

void Widget::moveBy(Widget::VarOff dx, Widget::VarOff dy)
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

void Widget::moveAt(Widget::VarDir argDir, Widget::VarOff argX, Widget::VarOff argY)
{
    m_dir = std::move(argDir);
    m_x   = std::make_pair(std::move(argX), 0);
    m_y   = std::make_pair(std::move(argY), 0);
}

Widget *Widget::disableSetSize()
{
    m_canSetSize = true; // can not flip back
    return this;
}

Widget *Widget::setW(Widget::VarSize argSize)
{
    if(m_canSetSize){
        throw fflerror("can not resize %s", name());
    }

    m_w = std::move(argSize);
    return this;
}

Widget *Widget::setH(Widget::VarSize argSize)
{
    if(m_canSetSize){
        throw fflerror("can not resize %s", name());
    }

    m_h = std::move(argSize);
    return this;
}

Widget *Widget::setSize(Widget::VarSize argW, Widget::VarSize argH)
{
    return setW(std::move(argW))->setH(std::move(argH));
}
