// label.hpp
#pragma once
#include <string>
#include <cmath>
#include <windows.h>
#include "../core/widget.hpp"
#include "../style.hpp"

class Label : public Widget {
public:
    enum class HAlign { Left, Center, Right };
    enum class VAlign { Top, Middle, Bottom };

    // properties
    std::wstring text;
    int padding = 2;                 // pixel padding (applied on all sides)
    std::wstring fontName = L"Segoe UI";
    double fontSizePt = 9.0;         // point size
    int fontWeight = FW_NORMAL;      // FW_NORMAL, FW_BOLD...
    bool italic = false;
    bool underline = false;
    bool strike = false;
    COLORREF textColor = RGB(0,0,0);
    COLORREF bgColor = RGB(255,255,255);
    bool transparentBg = true;

    HAlign hAlign = HAlign::Left;
    VAlign vAlign = VAlign::Top;

    bool wrap = false;               // word-wrap
    bool ellipsize = false;          // show ellipsis if single-line doesn't fit
    int maxLines = 0;                // 0 = unlimited (only meaningful when wrap = true)

private:
    HFONT hFont = nullptr;

public:
    Label(const std::wstring& t)
        : text(t)
    {
        focusable = false; // labels are not focusable by default
        createFont();
    }

    ~Label() override {
        if (hFont) DeleteObject(hFont);
    }

    // --- Setters that recreate font when needed ---
    void setFontName(const std::wstring& name) {
        fontName = name;
        recreateFont();
    }
    void setFontSize(double pt) {
        fontSizePt = pt;
        recreateFont();
    }
    void setBold(bool on) {
        fontWeight = on ? FW_BOLD : FW_NORMAL;
        recreateFont();
    }
    void setItalic(bool on) { italic = on; recreateFont(); }
    void setUnderline(bool on) { underline = on; recreateFont(); }
    void setStrike(bool on) { strike = on; recreateFont(); }

    void setPadding(int p) { padding = p; markLayoutDirty(); markDirty(); }
    void setTextColor(COLORREF c) { textColor = c; markDirty(); }
    void setBgColor(COLORREF c) { bgColor = c; markDirty(); transparentBg = false; }
    void setTransparentBg(bool t) { transparentBg = t; markDirty(); }

    void setHAlign(HAlign a) { hAlign = a; markDirty(); }
    void setVAlign(VAlign a) { vAlign = a; markDirty(); }

    void setWrap(bool w) { wrap = w; markLayoutDirty(); markDirty(); }
    void setEllipsize(bool e) { ellipsize = e; markLayoutDirty(); markDirty(); }
    void setMaxLines(int n) { maxLines = n; markLayoutDirty(); markDirty(); }

    // --- Drawing & measuring ---
    void draw(HDC hdc, int ox = 0, int oy = 0) override {
        // set font into the provided HDC (the actual painting DC)
        HFONT oldFont = (HFONT)SelectObject(hdc, hFont ? hFont : (HFONT)GetStockObject(DEFAULT_GUI_FONT));

        // background
        RECT r = {
            (int)(ox + rect.x),
            (int)(oy + rect.y),
            (int)(ox + rect.x + rect.w),
            (int)(oy + rect.y + rect.h)
        };
        if (!transparentBg) {
            HBRUSH br = CreateSolidBrush(bgColor);
            FillRect(hdc, &r, br);
            DeleteObject(br);
        }

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, textColor);

        // inner rect (apply padding)
        RECT ir = { r.left + padding, r.top + padding, r.right - padding, r.bottom - padding };

        // choose draw flags
        UINT flags = DT_NOPREFIX;
        if (wrap) {
            flags |= DT_WORDBREAK;
            if (maxLines > 0) {
                // DT_EDITCONTROL + setting bottom of rect to measured line count not trivial;
                // We'll rely on DrawText truncation (it will wrap and grow vertically up to rect bottom).
            }
        } else {
            flags |= DT_SINGLELINE;
            if (ellipsize) flags |= DT_END_ELLIPSIS;
        }

        // horizontal alignment
        if (hAlign == HAlign::Center) flags |= DT_CENTER;
        if (hAlign == HAlign::Right) flags |= DT_RIGHT;

        // vertical alignment: DT_VCENTER works only with DT_SINGLELINE or with exact rect height;
        if (!wrap) {
            if (vAlign == VAlign::Middle) flags |= DT_VCENTER;
            else if (vAlign == VAlign::Bottom) flags |= DT_BOTTOM;
            else flags |= DT_TOP;
        } else {
            // for multiline vertical align, we need to measure text height and compute offset
            // We'll measure and then draw with offset if needed.
            RECT measureRect = ir;
            DrawTextW(hdc, text.c_str(), (int)text.size(), &measureRect, flags | DT_CALCRECT);
            int textH = measureRect.bottom - measureRect.top;
            int availH = ir.bottom - ir.top;
            int dy = 0;
            if (vAlign == VAlign::Middle) dy = (availH - textH) / 2;
            else if (vAlign == VAlign::Bottom) dy = availH - textH;
            // cap
            if (dy < 0) dy = 0;
            // shift inner rect
            ir.top += dy;
            ir.bottom = ir.top + textH;
        }

        // finally draw text
        DrawTextW(hdc, text.c_str(), (int)text.size(), &ir, flags);

        SelectObject(hdc, oldFont);

        // draw children
        Widget::draw(hdc, ox, oy);
    }

    Size measure(const LayoutConstraints& c) const override {
        // create compatible DC for measurement (DPI of screen)
        HDC hdc = CreateCompatibleDC(nullptr);
        HFONT oldFont = (HFONT)SelectObject(hdc, hFont ? hFont : (HFONT)GetStockObject(DEFAULT_GUI_FONT));

        Size s{0,0};

        // constraints (cast to int safely)
        int maxW = (int)clampDouble(c.maxW, 0.0, 1000000.0);
        int maxH = (int)clampDouble(c.maxH, 0.0, 1000000.0);

        if (!wrap) {
            // single-line measurement: use GetTextExtentPoint32W for precise width/height
            SIZE ts{};
            if (!text.empty()) {
                GetTextExtentPoint32W(hdc, text.c_str(), (int)text.size(), &ts);
            } else {
                // empty string: measure typical height using TEXTMETRIC
                TEXTMETRIC tm{};
                GetTextMetrics(hdc, &tm);
                ts.cx = 0;
                ts.cy = tm.tmHeight;
            }
            LOG("Label single-line measure text w:" << ts.cx << " h:" << ts.cy);
            s.w = ts.cx + 2 * padding;
            s.h = ts.cy + 2 * padding;

            // if ellipsize is on and there is a max width constraint, clamp width to maxW
            if (ellipsize && maxW > 0) {
                s.w = clampDouble(s.w, c.minW, (double)maxW);
            }
        } else {
            // wrap: use DrawText with DT_CALCRECT and DT_WORDBREAK, giving it a width to wrap to.
            // If caller provided a finite maxW, use that. Otherwise, measure as single-line.
            RECT r = {0, 0, maxW > 0 ? maxW - 2*padding : 0, 0};
            UINT flags = DT_CALCRECT | DT_WORDBREAK | DT_NOPREFIX;
            if (maxLines > 0) {
                // no direct DT for maxLines measurement; we'll measure and then cap height by line count.
                // Measure full text height first.
            }
            if (r.right <= 0) {
                // no width limit: fall back to single-line
                SIZE ts{};
                if (!text.empty()) GetTextExtentPoint32W(hdc, text.c_str(), (int)text.size(), &ts);
                s.w = ts.cx + 2*padding;
                s.h = ts.cy + 2*padding;
            } else {
                DrawTextW(hdc, text.c_str(), (int)text.size(), &r, flags);
                int textW = r.right - r.left;
                int textH = r.bottom - r.top;
                // handle maxLines: estimate line height and cap
                if (maxLines > 0) {
                    TEXTMETRIC tm{};
                    GetTextMetrics(hdc, &tm);
                    int lineH = tm.tmHeight;
                    int capH = lineH * maxLines;
                    if (textH > capH) textH = capH;
                }
                s.w = textW + 2*padding;
                s.h = textH + 2*padding;
            }
        }

        // clamp to constraints
        s.w = clampDouble(s.w, c.minW, c.maxW);
        s.h = clampDouble(s.h, c.minH, c.maxH);

        SelectObject(hdc, oldFont);
        DeleteDC(hdc);

        LOG("Label measure w:" << s.w << " h:" << s.h);
        // store measured size if you want
        meansureSize = s;
        return s;
    }

private:
    void createFont() {
        // initial creation called by constructor
        if (hFont) { DeleteObject(hFont); hFont = nullptr; }

        HDC screen = GetDC(nullptr);
        int logpixely = GetDeviceCaps(screen, LOGPIXELSY);
        ReleaseDC(nullptr, screen);

        // height in logical units (negative to specify character height)
        int height = -MulDiv((int)std::round(fontSizePt), logpixely, 72);

        hFont = CreateFontW(
            height,
            0, 0, 0,
            fontWeight,
            italic ? TRUE : FALSE,
            underline ? TRUE : FALSE,
            strike ? TRUE : FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE,
            fontName.c_str()
        );
    }

    void recreateFont() {
        createFont();
        markLayoutDirty();
        markDirty();
    }
};
