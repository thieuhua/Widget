#pragma once
#include <string>
#include <windows.h>
#include "../core/widget.hpp"
#include "../style.hpp"

class Label : public Widget {
public:
    enum class HAlign { Left, Center, Right };
    enum class VAlign { Top, Middle, Bottom };

private:
    TextStyle style;
    std::wstring text;
    int padding = 2;
    HAlign hAlign = HAlign::Left;
    VAlign vAlign = VAlign::Top;
    bool wrap = false;
    bool ellipsize = false;
    int maxLines = 0;

public:
    Label(const std::wstring& t = L"", const TextStyle& st = {})
        : style(st), text(t)
    {}

    void setText(const std::wstring& t) { text = t; markDirty(); }
    void setPadding(int p) { padding = p; markLayoutDirty(); markDirty(); }

    TextStyle& getStyle() { return style; }
    const TextStyle& getStyle() const { return style; }

    // forward -> operator to style:
    TextStyle* operator->() { return &style; }

    void setHAlign(HAlign a) { hAlign = a; markDirty(); }
    void setVAlign(VAlign a) { vAlign = a; markDirty(); }
    void setWrap(bool w) { wrap = w; markLayoutDirty(); markDirty(); }
    void setEllipsize(bool e) { ellipsize = e; markLayoutDirty(); markDirty(); }
    void setMaxLines(int n) { maxLines = n; markLayoutDirty(); markDirty(); }

    // --- Draw ---
    void draw(HDC hdc, int ox = 0, int oy = 0) override {
        HFONT oldFont = (HFONT)SelectObject(hdc, style.getFont());
        RECT r = { (int)(ox + rect.x), (int)(oy + rect.y),
                   (int)(ox + rect.x + rect.w), (int)(oy + rect.y + rect.h) };

        if (!style.transparentBg) {
            HBRUSH br = CreateSolidBrush(style.bgColor);
            FillRect(hdc, &r, br);
            DeleteObject(br);
        }

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, style.textColor);

        RECT ir = { r.left + padding, r.top + padding, r.right - padding, r.bottom - padding };
        UINT flags = DT_NOPREFIX;

        if (wrap) {
            flags |= DT_WORDBREAK;
        } else {
            flags |= DT_SINGLELINE;
            if (ellipsize) flags |= DT_END_ELLIPSIS;
        }

        if (hAlign == HAlign::Center) flags |= DT_CENTER;
        else if (hAlign == HAlign::Right) flags |= DT_RIGHT;

        if (!wrap) {
            if (vAlign == VAlign::Middle) flags |= DT_VCENTER;
            else if (vAlign == VAlign::Bottom) flags |= DT_BOTTOM;
        } else {
            RECT measureRect = ir;
            DrawTextW(hdc, text.c_str(), (int)text.size(), &measureRect, flags | DT_CALCRECT);
            int textH = measureRect.bottom - measureRect.top;
            int availH = ir.bottom - ir.top;
            int dy = 0;
            if (vAlign == VAlign::Middle) dy = (availH - textH) / 2;
            else if (vAlign == VAlign::Bottom) dy = availH - textH;
            if (dy < 0) dy = 0;
            ir.top += dy;
        }

        DrawTextW(hdc, text.c_str(), (int)text.size(), &ir, flags);
        SelectObject(hdc, oldFont);

        Widget::draw(hdc, ox, oy);
    }

    // --- Measure ---
    Size measure(const LayoutConstraints& c) const override {
        HDC hdc = CreateCompatibleDC(nullptr);
        HFONT oldFont = (HFONT)SelectObject(hdc, style.getFont());

        Size s{0, 0};
        int maxW = (int)clampDouble(c.maxW, 0.0, 1000000.0);

        if (!wrap) {
            SIZE ts{};
            if (!text.empty())
                GetTextExtentPoint32W(hdc, text.c_str(), (int)text.size(), &ts);
            else {
                TEXTMETRIC tm{};
                GetTextMetrics(hdc, &tm);
                ts.cx = 0; ts.cy = tm.tmHeight;
            }
            s.w = ts.cx + 2 * padding;
            s.h = ts.cy + 2 * padding;
        } else {
            RECT r = { 0, 0, maxW > 0 ? maxW - 2 * padding : 0, 0 };
            UINT flags = DT_CALCRECT | DT_WORDBREAK | DT_NOPREFIX;
            DrawTextW(hdc, text.c_str(), (int)text.size(), &r, flags);
            s.w = (r.right - r.left) + 2 * padding;
            s.h = (r.bottom - r.top) + 2 * padding;
        }

        s.w = clampDouble(s.w, c.minW, c.maxW);
        s.h = clampDouble(s.h, c.minH, c.maxH);

        SelectObject(hdc, oldFont);
        DeleteDC(hdc);

        meansureSize = s;
        return s;
    }
};
