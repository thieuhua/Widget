#pragma once
#include <string>
#include <windows.h>
#include <cmath>

class TextStyle {
public:
    std::wstring fontName = L"Segoe UI";
    double fontSizePt = 9.0;
    int fontWeight = FW_NORMAL;
    bool italic = false;
    bool underline = false;
    bool strike = false;
    COLORREF textColor = RGB(0, 0, 0);
    COLORREF bgColor = RGB(255, 255, 255);
    bool transparentBg = true;

private:
    HFONT hFont = nullptr;

public:
    TextStyle() { createFont(); }

    TextStyle(const std::wstring& name, double pt = 9.0)
        : fontName(name), fontSizePt(pt)
    {
        createFont();
    }

    ~TextStyle() {
        if (hFont) DeleteObject(hFont);
    }

    HFONT getFont() const { return hFont; }

    void setFontName(const std::wstring& name) { fontName = name; recreateFont(); }
    void setFontSize(double pt) { fontSizePt = pt; recreateFont(); }
    void setBold(bool on) { fontWeight = on ? FW_BOLD : FW_NORMAL; recreateFont(); }
    void setItalic(bool on) { italic = on; recreateFont(); }
    void setUnderline(bool on) { underline = on; recreateFont(); }
    void setStrike(bool on) { strike = on; recreateFont(); }

    void setTextColor(COLORREF c) { textColor = c; }
    void setBgColor(COLORREF c) { bgColor = c; transparentBg = false; }
    void setTransparentBg(bool t) { transparentBg = t; }

private:
    void createFont() {
        if (hFont) { DeleteObject(hFont); hFont = nullptr; }

        HDC screen = GetDC(nullptr);
        int logpixely = GetDeviceCaps(screen, LOGPIXELSY);
        ReleaseDC(nullptr, screen);

        int height = -MulDiv((int)std::round(fontSizePt), logpixely, 72);

        hFont = CreateFontW(
            height, 0, 0, 0,
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
    }
};
