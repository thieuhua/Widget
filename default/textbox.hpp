#pragma  once
#include <string>
#include <cmath>
#include <windows.h>
#include "../core/widget.hpp"

class TextBox : public Widget {
public:
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
    bool transparentBg = false;
private:
    HFONT hFont = nullptr;
public:
    TextBox(const std::wstring& t)
        : text(t)
    {
        createFont();
    }
    ~TextBox() override {
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