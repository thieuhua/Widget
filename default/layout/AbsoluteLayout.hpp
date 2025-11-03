#pragma one
#include "../core/layout.hpp"
#include "../core/widget.hpp"


// AbsoluteLayout
class AbsoluteLayout : public Layout {
public:
    Size measure(Widget* widget, const LayoutConstraints& c) override {
        // measure children if needed; default: prefer container current size
        double w = 0, h = 0;
        for (auto &childPtr : widget->children) {
            Widget* child = childPtr.get();
            Size s = child->measure(LayoutConstraints::Unbounded());
            w = std::max(w, child->rect.x + s.w);
            h = std::max(h, child->rect.y + s.h);
        }
        LOG("AbsoluteLayout measure w:" << w << " h:" << h);
        return { clampDouble(w, c.minW, c.maxW), clampDouble(h, c.minH, c.maxH) };
    }

    void arrange(Widget* widget, const Rect& bounds) override {
        LOG("AbsoluteLayout arrange in bounds x:" << bounds.x << " y:" << bounds.y << " w:" << bounds.w << " h:" << bounds.h);
        for (auto &childPtr : widget->children) {
            Widget* child = childPtr.get();
            // absolute positions, just add parent offset
            Size measure = child->meansureSize;
            Rect r {child->rect.x, child->rect.y, measure.w, measure.h };
            LOG("  arranging child at x:" << r.x << " y:" << r.y << " w:" << r.w << " h:" << r.h);
            child->arrange(r);
        }
    }
};