#pragma one
#include "../core/layout.hpp"
#include "../core/widget.hpp"


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
            Size p = child->measure(cc);
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
        // compute cell sizes (uniform from measure step)
        double totalSpacingW = spacing * (cols - 1) + 2*padding;
        double totalSpacingH = spacing * (rows - 1) + 2*padding;
        double cellW = (bounds.w - totalSpacingW) / cols;
        double cellH = (bounds.h - totalSpacingH) / rows;

        for (size_t i = 0; i < widget->children.size(); ++i) {
            int r = (int)(i / cols);
            int col = (int)(i % cols);
            double x =padding + col * (cellW + spacing);
            double y =padding + r * (cellH + spacing);
            Rect cr { x, y, cellW, cellH };
            Widget* child = widget->children[i].get();
            child->arrange(cr);
        }
    }
};
