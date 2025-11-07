struct ROI final
{
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    bool empty() const noexcept
    {
        return w <= 0 || h <= 0;
    }

    operator bool () const noexcept
    {
        return !empty();
    }

    bool in(int argX, int argY) const noexcept
    {
        return mathf::pointInRectangle<int>(argX, argY, x, y, w, h);
    }

    bool overlap(const Widget::ROI &r) const noexcept
    {
        return mathf::rectangleOverlap<int>(x, y, w, h, r.x, r.y, r.w, r.h);
    }

    Widget::ROI clone() const noexcept
    {
        return *this;
    }

    Widget::ROI & crop(const Widget::ROI &r)
    {
        mathf::cropSegment<int>(x, w, r.x, r.w);
        mathf::cropSegment<int>(y, h, r.y, r.h);
        return *this;
    }
};

struct VarROI final
{
    Widget::VarInt  x = 0;
    Widget::VarInt  y = 0;
    Widget::VarSize w = 0;
    Widget::VarSize h = 0;

    Widget::ROI roi(const Widget *widget, const void *arg) const
    {
        return
        {
            .x = Widget::evalInt (x, widget, arg),
            .y = Widget::evalInt (y, widget, arg),
            .w = Widget::evalSize(w, widget, arg),
            .h = Widget::evalSize(h, widget, arg),
        };
    }
};

class VarROIOpt final
{
    private:
        std::optional<Widget::VarROI> m_varROIOpt;

    public:
        VarROIOpt() = default;
        VarROIOpt(std::nullopt_t): VarROIOpt() {};

    public:
        VarROIOpt(Widget::VarSize argW, Widget::VarSize argH)
            : VarROIOpt(0, 0, std::move(argW), std::move(argH))
        {}

        VarROIOpt(Widget::VarInt argX, Widget::VarInt argY, Widget::VarSize argW, Widget::VarSize argH)
            : m_varROIOpt(Widget::VarROI{std::move(argX), std::move(argY), std::move(argW), std::move(argH)})
        {}

    public:
        VarROIOpt(const Widget::ROI &r)
            : VarROIOpt(r.x, r.y, r.w, r.h)
        {}

        VarROIOpt(const Widget::VarROI &vr)
            : m_varROIOpt(vr)
        {}

    public:
        auto operator -> (this auto && self)
        {
            return std::addressof(self.m_varROIOpt.value());
        }

    public:
        bool has_value() const
        {
            return m_varROIOpt.has_value();
        }

        decltype(auto) value(this auto && self)
        {
            return self.m_varROIOpt.value();
        }

        Widget::VarROI value_or(Widget::VarROI r) const
        {
            return m_varROIOpt.value_or(r);
        }
};

class ROIOpt final
{
    private:
        std::optional<Widget::ROI> m_roiOpt;

    public:
        ROIOpt() = default;
        ROIOpt(std::nullopt_t): ROIOpt() {};

    public:
        ROIOpt(int argW, int argH)
            : ROIOpt(0, 0, argW, argH)
        {}

        ROIOpt(int argX, int argY, int argW, int argH)
            : m_roiOpt(Widget::ROI{argX, argY, argW, argH})
        {}

    public:
        ROIOpt(const Widget::ROI &roi)
            : m_roiOpt(roi)
        {}

    public:
        auto operator -> (this auto && self)
        {
            return std::addressof(self.m_roiOpt.value());
        }

    public:
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

    Widget::ROIMap & calibrate(const Widget *widget)
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

        return *this;
    }

    Widget::ROIMap & crop(const Widget::ROI &r)
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

        return *this;
    }

    Widget::ROIMap clone() const
    {
        return *this;
    }

    Widget::ROIMap create(const Widget::ROI &cr /* child ROI */) const
    {
        auto r = *this;
        r.crop(cr);

        r.ro->x -= cr.x;
        r.ro->y -= cr.y;

        return r;
    }

    bool empty() const
    {
        if(ro.has_value()){
            return ro->empty();
        }
        else{
            throw fflerror("ro empty");
        }
    }

    operator bool () const
    {
        return !empty();
    }

    bool in(int pixelX, int pixelY) const
    {
        if(ro.has_value()){
            return Widget::ROI{x, y, ro->w, ro->h}.in(pixelX, pixelY);
        }
        else{
            throw fflerror("ro empty");
        }
    }

    template<typename T> bool in(const T &t) const
    {
        const auto [tx, ty] = t; return in(tx, ty);
    }
};

template<typename T> Widget::ROI makeROI(const T &t)
{
    const auto [x, y, w, h] = t; return Widget::ROI
    {
        .x = x,
        .y = y,
        .w = w,
        .h = h,
    };
}

template<typename U, typename V> Widget::ROI makeROI(const U &u, const V &v)
{
    const auto [x, y] = u;
    const auto [w, h] = v; return Widget::ROI
    {
        .x = x,
        .y = y,
        .w = w,
        .h = h,
    };
}

template<typename T> Widget::ROI makeROI(int x, int y, const T &t)
{
    const auto [w, h] = t; return Widget::ROI
    {
        .x = x,
        .y = y,
        .w = w,
        .h = h,
    };
}

template<typename T> Widget::ROI makeROI(const T &t, int w, int h)
{
    const auto [x, y] = t; return Widget::ROI
    {
        .x = x,
        .y = y,
        .w = w,
        .h = h,
    };
}
