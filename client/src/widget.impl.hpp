auto WidgetTreeNode::parent(this auto && self, unsigned level) -> check_const_cond_out_ptr_t<decltype(self), Widget>
{
    check_const_cond_out_ptr_t<decltype(self), Widget> p = std::addressof(self);
    for(; p && (level > 0); level--){
        p = p->m_parent;
    }
    return p;
}

template<std::invocable<const Widget *, bool, const Widget *, bool> F> void WidgetTreeNode::sort(F f)
{
    m_childList.sort(m_childList.begin(), m_childList.end(), [&f](const auto &x, const auto &y)
    {
        if(x.widget && y.widget){
            return f(x.widget, x.autoDelete, y.widget, y.autoDelete);
        }
        else if(x.widget){
            return true;
        }
        else{
            return false;
        }
    });
}

template<typename SELF> auto WidgetTreeNode::foreachChild(this SELF && self, bool forward, std::invocable<check_const_cond_out_ptr_t<SELF, Widget>, bool> auto f) -> std::conditional_t<std::is_same_v<std::invoke_result_t<decltype(f), Widget *, bool>, bool>, bool, void>
{
    const ValueKeeper keepValue(self.m_inLoop, true);
    constexpr bool hasBoolResult = std::is_same_v<std::invoke_result_t<decltype(f), Widget *, bool>, bool>;

    if(forward){
        for(auto p = self.m_childList.begin(); p != self.m_childList.end(); ++p){
            if(p->widget){
                if constexpr (hasBoolResult){
                    if(f(p->widget, p->autoDelete)){
                        return true;
                    }
                }
                else{
                    f(p->widget, p->autoDelete);
                }
            }
        }

        if constexpr (hasBoolResult){
            return false;
        }
    }
    else{
        for(auto p = self.m_childList.rbegin(); p != self.m_childList.rend(); ++p){
            if(p->widget){
                if constexpr (hasBoolResult){
                    if(f(p->widget, p->autoDelete)){
                        return true;
                    }
                }
                else{
                    f(p->widget, p->autoDelete);
                }
            }
        }

        if constexpr (hasBoolResult){
            return false;
        }
    }
}

template<typename SELF> auto WidgetTreeNode::foreachChild(this SELF && self, std::invocable<check_const_cond_out_ptr_t<SELF, Widget>, bool> auto f) -> std::conditional_t<std::is_same_v<std::invoke_result_t<decltype(f), Widget *, bool>, bool>, bool, void>
{
    if constexpr (std::is_same_v<std::invoke_result_t<decltype(f), check_const_cond_out_ptr_t<SELF, Widget>, bool>, bool>){
        return self.foreachChild(true, f);
    }
    else{
        self.foreachChild(true, f);
    }
}

auto WidgetTreeNode::firstChild(this auto && self) -> check_const_cond_out_ptr_t<decltype(self), Widget>
{
    for(auto &child: self.m_childList){
        if(child.widget){
            return child.widget;
        }
    }
    return nullptr;
}

auto WidgetTreeNode::lastChild(this auto && self) -> check_const_cond_out_ptr_t<decltype(self), Widget>
{
    for(auto p = self.m_childList.rbegin(); p != self.m_childList.rend(); ++p){
        if(p->widget){
            return p->widget;
        }
    }
    return nullptr;
}

void WidgetTreeNode::clearChild(std::invocable<const Widget *, bool> auto f)
{
    for(auto &child: m_childList){
        if(child.widget){
            if(f(child.widget, child.autoDelete)){
                removeChildElement(child, true);
            }
        }
    }
}

auto WidgetTreeNode::hasChild(this auto && self, uint64_t argID) -> check_const_cond_out_ptr_t<decltype(self), Widget>
{
    for(auto p = self.m_childList.begin(); p != self.m_childList.end(); ++p){
        if(p->widget && p->widget->id() == argID){
            return p->widget;
        }
    }
    return nullptr;
}

auto WidgetTreeNode::hasChild(this auto && self, std::invocable<const Widget *, bool> auto f) -> check_const_cond_out_ptr_t<decltype(self), Widget>
{
    for(auto &child: self.m_childList){
        if(child.widget && f(child.widget, child.autoDelete)){
            return child.widget;
        }
    }
    return nullptr;
}

auto WidgetTreeNode::hasDescendant(this auto && self, uint64_t argID) -> check_const_cond_out_ptr_t<decltype(self), Widget>
{
    for(auto p = self.m_childList.begin(); p != self.m_childList.end(); ++p){
        if(p->widget){
            if(p->widget->id() == argID){
                return p->widget;
            }
            else if(auto descendant = p->widget->hasDescendant(argID)){
                return descendant;
            }
        }
    }
    return nullptr;
}

auto WidgetTreeNode::hasDescendant(this auto && self, std::invocable<const Widget *, bool> auto f) -> check_const_cond_out_ptr_t<decltype(self), Widget>
{
    for(auto &child: self.m_childList){
        if(child.widget){
            if(f(child.widget, child.autoDelete)){
                return child.widget;
            }
            else if(auto descendant = child.widget->hasDescendant(f)){
                return descendant;
            }
        }
    }
    return nullptr;
}

template<std::derived_from<Widget> T> auto WidgetTreeNode::hasParent(this auto && self) -> check_const_cond_out_ptr_t<decltype(self), T>
{
    for(auto p = self.parent(); p; p = p->parent()){
        if constexpr (std::is_const_v<std::remove_reference_t<decltype(self)>>){
            if(dynamic_cast<const T *>(p)){
                return static_cast<const T *>(p);
            }
        }
        else{
            if(dynamic_cast<T *>(p)){
                return static_cast<T *>(p);
            }
        }
    }
    return nullptr;
}

template<typename T> Widget::ROI Widget::makeROI(const T &t)
{
    const auto [x, y, w, h] = t;
    return Widget::ROI
    {
        .x = x,
        .y = y,
        .w = w,
        .h = h,
    };
}

int Widget::evalSizeOpt(const Widget::VarSizeOpt &varSizeOpt, const Widget *widget, const auto &f)
{
    if(varSizeOpt.has_value()){
        return evalSize(varSizeOpt.value(), widget, nullptr);
    }
    else{
        return f();
    }
}

int Widget::evalSizeOpt(const Widget::VarSizeOpt &varSizeOpt, const Widget *widget, const void *arg, const auto &f)
{
    if(varSizeOpt.has_value()){
        return evalSize(varSizeOpt.value(), widget, arg);
    }
    else{
        return f();
    }
}

template<typename Func> Widget::VarSize Widget::transform(Widget::VarSize varSize, Func && func)
{
    if(varSize.index() == 0){
        return func(std::get<int>(varSize));
    }
    else{
        return [varSize = std::move(varSize), func = std::forward<Func>(func)](const Widget *widget)
        {
            return func(std::get<std::function<int(const Widget *)>>(varSize)(widget));
        };
    }
}

template<typename Func> Widget::VarSizeOpt Widget::transform(Widget::VarSizeOpt varSize, Func && func)
{
    if(varSize.has_value()){
        return transform(varSize.value(), std::forward<Func>(func));
    }
    else{
        return std::nullopt;
    }
}

auto Widget::focusedChild(this auto && self) -> check_const_cond_out_ptr_t<decltype(self), Widget>
{
    if(self.firstChild() && self.firstChild()->focus()){
        return self.firstChild();
    }
    return nullptr;
}
