#pragma one
#include "../core/layout.hpp"
#include "../core/widget.hpp"

// FlowLayout
class FlowLayout : public Layout {
public:
    double spacing = 4.0;
    double lineSpacing = 4.0;
    double padding = 4.0;
    bool wrap = true;

    Size measure(Widget* widget, const LayoutConstraints& c) override {

        double x = padding-spacing, y = padding, maxHeightLine = 0;
        double totalW = 0, totalH = 0;
        LOG("FlowLayout measure with Width:" << c.maxW);

        for (auto &childPtr : widget->children) {
            Widget* child = childPtr.get();
            Size s =child->measure(LayoutConstraints::Unbounded());

            if (wrap && x + s.w  + padding> c.maxW) {
                x = padding - spacing;
                y += maxHeightLine + lineSpacing;
                maxHeightLine = 0;
            }
            x += spacing + s.w;
            maxHeightLine = std::max(maxHeightLine, s.h);
            totalW = std::max(totalW, x+padding);
            totalH = y + maxHeightLine;
        }
        totalH += lineSpacing; // bottom spacing
        LOG("FlowLayout measure totalW:" << totalW << " totalH:" << totalH);
        return { clampDouble(totalW, c.minW, c.maxW), clampDouble(totalH, c.minH, c.maxH) };
    }

    void arrange(Widget* widget, const Rect& bounds) override {
        LOG("FlowLayout arrange in bounds x:" << bounds.x << " y:" << bounds.y << " w:" << bounds.w << " h:" << bounds.h);
        double x = spacing, y = lineSpacing;
        double lineHeight = 0;

        for (auto &childPtr : widget->children) {
            Widget* child = childPtr.get();
            Size s = childPtr->meansureSize;

            if (wrap && x + s.w > bounds.w) {
                x = spacing;
                y += lineHeight + lineSpacing;
                lineHeight = 0;
            }


            Rect r { x, y, s.w, s.h };
            child->arrange(r);

            x += s.w + spacing;
            lineHeight = std::max(lineHeight, s.h);
        }
    }
};