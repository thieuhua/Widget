#include "../widget.hpp"

class Panel : public Widget {
protected:
    COLORREF bgColor = RGB(240, 240, 240);
public:
    Panel() = default;
    void setBackground(COLORREF color) { bgColor = color; markDirty(); }

    void draw(HDC hdc, int ox=0, int oy=0) override {
        RECT r = {(LONG)(ox + rect.x), (LONG)(oy + rect.y),
                  (LONG)(ox + rect.x + rect.w), (LONG)(oy + rect.y + rect.h)};
        HBRUSH brush = CreateSolidBrush(bgColor);
        FillRect(hdc, &r, brush);
        DeleteObject(brush);

        // Draw children
        for (auto &c : children)
            c->draw(hdc, ox + (int)rect.x, oy + (int)rect.y);
    }
};

enum class Orientation { Vertical, Horizontal };

class StackPanel : public Panel {
    Orientation orientation = Orientation::Vertical;
    double spacing = 4.0;
public:
    StackPanel(Orientation o = Orientation::Vertical) : orientation(o) {}

    void setSpacing(double s) { spacing = s; markDirty(); }

    void updateLayout() {
        double offset = 0;
        for (auto &c : children) {
            if (orientation == Orientation::Vertical) {
                c->rect.y = offset;
                offset += c->rect.h + spacing;
            } else {
                c->rect.x = offset;
                offset += c->rect.w + spacing;
            }
        }
    }

    void update(double dt) override {
        updateLayout();
        Panel::update(dt);
    }
};

class GridPanel : public Panel {
    int rows = 1;
    int cols = 1;
    double padding = 2.0;
public:
    GridPanel(int r = 1, int c = 1) : rows(r), cols(c) {}

    void setGrid(int r, int c) { rows = r; cols = c; markDirty(); }

    void updateLayout() {
        double cellW = rect.w / cols;
        double cellH = rect.h / rows;

        for (size_t i = 0; i < children.size(); ++i) {
            int row = (int)(i / cols);
            int col = (int)(i % cols);
            Widget* c = children[i].get();

            c->rect.x = col * cellW + padding;
            c->rect.y = row * cellH + padding;
            c->rect.w = cellW - 2 * padding;
            c->rect.h = cellH - 2 * padding;
        }
    }

    void update(double dt) override {
        updateLayout(); 
        Panel::update(dt);
    }
};

class ScrollPanel : public Panel {
    double scrollY = 0.0;
    double contentHeight = 0.0;
public:
    void scroll(double dy) {
        scrollY += dy;
        if (scrollY < 0) scrollY = 0;
        if (scrollY > contentHeight - rect.h)
            scrollY = std::max(0.0, contentHeight - rect.h);
        markDirty();
    }

    void update(double dt) override {
        // tính chiều cao nội dung
        double maxY = 0;
        for (auto &c : children)
            maxY = std::max(maxY, c->rect.y + c->rect.h);
        contentHeight = maxY;
        Panel::update(dt);
    }

    void draw(HDC hdc, int ox=0, int oy=0) override {
        RECT clip = {(LONG)(ox + rect.x), (LONG)(oy + rect.y),
                     (LONG)(ox + rect.x + rect.w), (LONG)(oy + rect.y + rect.h)};
        SaveDC(hdc);
        IntersectClipRect(hdc, clip.left, clip.top, clip.right, clip.bottom);

        for (auto &c : children)
            c->draw(hdc, ox + (int)rect.x, oy + (int)(rect.y - scrollY));

        RestoreDC(hdc, -1);
    }

    void onMouseMove(int, int y) override {
        // ví dụ: cuộn bằng kéo chuột (chưa hoàn chỉnh)
        // có thể thêm xử lý WM_MOUSEWHEEL ở cấp cao hơn
    }
};
