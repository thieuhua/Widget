#include <string>
#include <windows.h>
#include "../widget.hpp"
#include "../layout.hpp"

class Label : public Widget {
public:
    std::wstring text;
    int padding = 2;
    HFONT font = nullptr; // nullptr = default system font

    Label(const std::wstring& t) : text(t) {}

    void draw(HDC hdc, int ox = 0, int oy = 0) override {
        TextOutW(hdc, (int)(ox + rect.x), (int)(oy + rect.y), text.c_str(), (int)text.size());
        Widget::draw(hdc, ox, oy);
    }

    // --- Measure size based on text ---
    Size measure(const LayoutConstraints& c) override {
        HDC hdc = GetDC(nullptr);
        HFONT oldFont = (HFONT)SelectObject(hdc, font ? font : (HFONT)GetStockObject(DEFAULT_GUI_FONT));
        RECT r = {0,0,0,0};
        DrawTextW(hdc, text.c_str(), (int)text.size(), &r, DT_CALCRECT | DT_SINGLELINE);
        SelectObject(hdc, oldFont);
        ReleaseDC(nullptr, hdc);

        Size s;
        s.w = r.right - r.left + 2 * padding;
        s.h = r.bottom - r.top + 2 * padding;
        // clamp to constraints
        s.w = std::clamp(s.w, c.minW, c.maxW);
        s.h = std::clamp(s.h, c.minH, c.maxH);
        return s;
    }

    void arrange(const Rect& bounds) override {
        rect = bounds;
        // no children to layout
    }
};
