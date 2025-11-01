#include "../core/layout.hpp"
#include "../core/widget.hpp"
// -------------------------
// detailed layouts
// -------------------------

// stack layout: vertical or horizontal stacking
class StackLayout : public Layout {
public:
    Orientation orientation = Orientation::Vertical;
    double spacing = 4.0;
    Align crossAlign = Align::Stretch; // how children align on perpendicular axis
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
            // if child has its own layout, delegate measurement to it
            Size pref;
            if (child->layout) pref = child->layout->measure(child, childConst);
            else pref = child->measure(childConst);

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
        // set widget bounds (the container itself)
        widget->rect = bounds;

        // compute area for children (inside padding)
        double x0 = bounds.x + padding;
        double y0 = bounds.y + padding;
        double availW = bounds.w - 2 * padding;
        double availH = bounds.h - 2 * padding;

        // measure children again with available cross axis size
        std::vector<Size> prefs;
        prefs.reserve(widget->children.size());
        for (auto &childPtr : widget->children) {
            Widget* child = childPtr.get();
            LayoutConstraints cc = LayoutConstraints::Unbounded();
            if (orientation == Orientation::Vertical) {
                cc.minW = 0; cc.maxW = availW;
            } else {
                cc.minH = 0; cc.maxH = availH;
            }
            Size p = child->layout ? child->layout->measure(child, cc) : child->measure(cc);
            prefs.push_back(p);
        }

        // place children
        double offset = 0;
        for (size_t i = 0; i < widget->children.size(); ++i) {
            Widget* child = widget->children[i].get();
            Size p = prefs[i];
            Rect cr;
            if (orientation == Orientation::Vertical) {
                cr.x = x0;
                cr.y = y0 + offset;
                cr.w = (crossAlign == Align::Stretch) ? availW : p.w;
                // horizontal alignment
                if (crossAlign == Align::Center) cr.x += (availW - cr.w) / 2.0;
                else if (crossAlign == Align::End) cr.x += (availW - cr.w);
                cr.h = p.h;
                offset += p.h + spacing;
            } else {
                cr.x = x0 + offset;
                cr.y = y0;
                cr.h = (crossAlign == Align::Stretch) ? availH : p.h;
                if (crossAlign == Align::Center) cr.y += (availH - cr.h) / 2.0;
                else if (crossAlign == Align::End) cr.y += (availH - cr.h);
                cr.w = p.w;
                offset += p.w + spacing;
            }

            // arrange child (if child has its own layout, that layout will arrange its children)
            if (child->layout) child->layout->arrange(child, cr);
            else child->arrange(cr);
        }
    }
};



// GridLayout (uniform grid): 
class GridLayout : public Layout {
public:
    int rows = 1;
    int cols = 1;
    double padding = 0;
    double spacing = 2.0;

    GridLayout(int r = 1, int c = 1) : rows(std::max(1, r)), cols(std::max(1, c)) {}

    Size measure(Widget* widget, const LayoutConstraints& c) override {
        // simple approach: measure children and compute max per column/row
        std::vector<double> colW(cols, 0.0);
        std::vector<double> rowH(rows, 0.0);

        for (size_t i = 0; i < widget->children.size(); ++i) {
            int r = (int)(i / cols);
            int col = (int)(i % cols);
            Widget* child = widget->children[i].get();
            LayoutConstraints cc;
            cc.maxW = c.maxW / cols;
            cc.maxH = c.maxH / rows;
            Size p = child->layout ? child->layout->measure(child, cc) : child->measure(cc);
            colW[col] = std::max(colW[col], p.w);
            rowH[r] = std::max(rowH[r], p.h);
        }

        Size out;
        for (double w : colW) out.w += w;
        for (double h : rowH) out.h += h;

        out.w += 2*padding + spacing * (cols - 1);
        out.h += 2*padding + spacing * (rows - 1);

        out.w = clampDouble(out.w, c.minW, c.maxW);
        out.h = clampDouble(out.h, c.minH, c.maxH);
        return out;
    }

    void arrange(Widget* widget, const Rect& bounds) override {
        widget->rect = bounds;
        // compute cell sizes (uniform from measure step)
        double totalSpacingW = spacing * (cols - 1) + 2*padding;
        double totalSpacingH = spacing * (rows - 1) + 2*padding;
        double cellW = (bounds.w - totalSpacingW) / cols;
        double cellH = (bounds.h - totalSpacingH) / rows;

        for (size_t i = 0; i < widget->children.size(); ++i) {
            int r = (int)(i / cols);
            int col = (int)(i % cols);
            double x = bounds.x + padding + col * (cellW + spacing);
            double y = bounds.y + padding + r * (cellH + spacing);
            Rect cr { x, y, cellW, cellH };
            Widget* child = widget->children[i].get();
            if (child->layout) child->layout->arrange(child, cr);
            else child->arrange(cr);
        }
    }
};


// DockLayout
enum class Dock { Left, Top, Right, Bottom, Fill };

class DockLayout : public Layout {
public:
    struct DockChild {
        Widget* widget;
        Dock dock;
    };

    std::vector<DockChild> dockChildren;

    void addDockedWidget(Widget* w, Dock d) {
        dockChildren.push_back({w,d});
    }

    Size measure(Widget* widget, const LayoutConstraints& c) override {
        // measure children independently; container prefers max of remaining space
        double totalW = 0, totalH = 0;
        for (auto &dc : dockChildren) {
            LayoutConstraints cc = c; // unconstrained
            Size s = dc.widget->layout ? dc.widget->layout->measure(dc.widget, cc) : dc.widget->measure(cc);
            totalW = std::max(totalW, s.w);
            totalH = std::max(totalH, s.h);
        }
        return { clampDouble(totalW, c.minW, c.maxW), clampDouble(totalH, c.minH, c.maxH) };
    }

    void arrange(Widget* widget, const Rect& bounds) override {
        Rect rem = bounds;

        for (auto &dc : dockChildren) {
            Widget* w = dc.widget;
            Rect r;
            Size pref = w->layout ? w->layout->measure(w, LayoutConstraints::Unbounded()) : w->measure(LayoutConstraints::Unbounded());

            switch(dc.dock) {
                case Dock::Left:
                    r = { rem.x, rem.y, pref.w, rem.h };
                    rem.x += pref.w;
                    rem.w -= pref.w;
                    break;
                case Dock::Right:
                    r = { rem.x + rem.w - pref.w, rem.y, pref.w, rem.h };
                    rem.w -= pref.w;
                    break;
                case Dock::Top:
                    r = { rem.x, rem.y, rem.w, pref.h };
                    rem.y += pref.h;
                    rem.h -= pref.h;
                    break;
                case Dock::Bottom:
                    r = { rem.x, rem.y + rem.h - pref.h, rem.w, pref.h };
                    rem.h -= pref.h;
                    break;
                case Dock::Fill:
                    r = rem;
                    rem = {0,0,0,0}; // no remaining
                    break;
            }
            if (w->layout) w->layout->arrange(w, r);
            else w->arrange(r);
        }
    }
};

// FlowLayout
class FlowLayout : public Layout {
public:
    double spacing = 4.0;
    double lineSpacing = 4.0;
    bool wrap = true;

    Size measure(Widget* widget, const LayoutConstraints& c) override {
        double x = 0, y = 0, maxHeightLine = 0;
        double totalW = 0, totalH = 0;

        for (auto &childPtr : widget->children) {
            Widget* child = childPtr.get();
            Size s = child->layout ? child->layout->measure(child, LayoutConstraints::Unbounded()) : child->measure(LayoutConstraints::Unbounded());

            if (wrap && x + s.w > c.maxW) {
                x = 0;
                y += maxHeightLine + lineSpacing;
                maxHeightLine = 0;
            }
            x += s.w + spacing;
            maxHeightLine = std::max(maxHeightLine, s.h);
            totalW = std::max(totalW, x);
            totalH = y + maxHeightLine;
        }
        return { clampDouble(totalW, c.minW, c.maxW), clampDouble(totalH, c.minH, c.maxH) };
    }

    void arrange(Widget* widget, const Rect& bounds) override {
        double x = bounds.x, y = bounds.y;
        double lineHeight = 0;

        for (auto &childPtr : widget->children) {
            Widget* child = childPtr.get();
            Size s = child->layout ? child->layout->measure(child, LayoutConstraints::Unbounded()) : child->measure(LayoutConstraints::Unbounded());

            if (wrap && x + s.w > bounds.x + bounds.w) {
                x = bounds.x;
                y += lineHeight + lineSpacing;
                lineHeight = 0;
            }

            Rect r { x, y, s.w, s.h };
            if (child->layout) child->layout->arrange(child, r);
            else child->arrange(r);

            x += s.w + spacing;
            lineHeight = std::max(lineHeight, s.h);
        }
    }
};

// -------------------------
// AbsoluteLayout
// -------------------------
class AbsoluteLayout : public Layout {
public:
    Size measure(Widget* widget, const LayoutConstraints& c) override {
        // measure children if needed; default: prefer container current size
        double w = 0, h = 0;
        for (auto &childPtr : widget->children) {
            Widget* child = childPtr.get();
            Size s = child->layout ? child->layout->measure(child, LayoutConstraints::Unbounded()) : child->measure(LayoutConstraints::Unbounded());
            w = std::max(w, child->rect.x + s.w);
            h = std::max(h, child->rect.y + s.h);
        }
        return { clampDouble(w, c.minW, c.maxW), clampDouble(h, c.minH, c.maxH) };
    }

    void arrange(Widget* widget, const Rect& bounds) override {
        widget->rect = bounds;
        for (auto &childPtr : widget->children) {
            Widget* child = childPtr.get();
            // absolute positions, just add parent offset
            Rect r = child->rect;
            r.x += bounds.x;
            r.y += bounds.y;
            if (child->layout) child->layout->arrange(child, r);
            else child->arrange(r);
        }
    }
};