#pragma one
#include "../core/layout.hpp"
#include "../core/widget.hpp"

// stack layout: vertical or horizontal stacking
class StackLayout : public Layout {
public:
    Orientation orientation = Orientation::Vertical;
    double spacing = 4.0;
    Align crossAlign = Align::Stretch; 
    double padding = 0.0;

    StackLayout() = default;
    StackLayout(Orientation o) : orientation(o) {}

    Size measure(Widget* widget, const LayoutConstraints& c) override {
        // compute content constraints for children
        Size result;
        double mainAvail = (orientation == Orientation::Vertical) ? c.maxH : c.maxW;
        // we'll pass unconstrained in main axis but constrained in cross axis
        for (auto &childPtr : widget->children) {
            Widget* child = childPtr.get();
            LayoutConstraints childConst = LayoutConstraints::Unbounded();
            if (orientation == Orientation::Vertical) {
                childConst.minW = c.minW;
                childConst.maxW = c.maxW;
            } else {
                childConst.minH = c.minH;
                childConst.maxH = c.maxH;
            }
            Size pref = child->measure(childConst);

            if (orientation == Orientation::Vertical) {
                result.h += pref.h;
                result.w = std::max(result.w, pref.w);
            } else {
                result.w += pref.w;
                result.h = std::max(result.h, pref.h);
            }
        }
        // add spacing
        if (!widget->children.empty()) {
            if (orientation == Orientation::Vertical) result.h += spacing * (widget->children.size() - 1);
            else result.w += spacing * (widget->children.size() - 1);
        }
        // add padding
        if (orientation == Orientation::Vertical) {
            result.h += 2 * padding;
            result.w += 2 * padding;
        } else {
            result.w += 2 * padding;
            result.h += 2 * padding;
        }

        // clamp to constraints
        result.w = clampDouble(result.w, c.minW, c.maxW);
        result.h = clampDouble(result.h, c.minH, c.maxH);
        return result;
    }

    void arrange(Widget* widget, const Rect& bounds) override {

        // compute area for children (inside padding)
        double x0 = padding;
        double y0 = padding;
        double availW = bounds.w - 2 * padding;
        double availH = bounds.h - 2 * padding;


        // place children
        double offset = 0;
        for (auto& childPtr: widget->children) {
            Size s = childPtr->meansureSize;
            Rect cr;
            if (orientation == Orientation::Vertical) {
                cr.x = x0;
                cr.y = y0 + offset;
                cr.w = (crossAlign == Align::Stretch) ? availW : s.w;
                // horizontal alignment
                if (crossAlign == Align::Center) cr.x += (availW - cr.w) / 2.0;
                else if (crossAlign == Align::End) cr.x += (availW - cr.w);
                cr.h = s.h;
                offset += s.h + spacing;
            } else {
                cr.x = x0 + offset;
                cr.y = y0;
                cr.h = (crossAlign == Align::Stretch) ? availH : s.h;
                if (crossAlign == Align::Center) cr.y += (availH - cr.h) / 2.0;
                else if (crossAlign == Align::End) cr.y += (availH - cr.h);
                cr.w = s.w;
                offset += s.w + spacing;
            }

            // arrange child (if child has its own layout, that layout will arrange its children)
            childPtr->arrange(cr);
        }
    }
};