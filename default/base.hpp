#include "../widget.hpp"
#include <functional>

class Button: public Widget {
    std::function<void()> onClickHandler;
public:
    Button(Rect rect) {
        this->rect = rect;
    }
    Button(int x, int y, int w, int h) {
        this->rect = Rect{(double)x, (double)y, (double)w, (double)h};
    }
    void setOnClickHandler(std::function<void()> handler) {
        onClickHandler = handler;
    }
    void draw(HDC hdc, int ox = 0, int oy = 0) override {
        RECT r;
        r.left = ox + (LONG)rect.x;
        r.top = oy + (LONG)rect.y;
        r.right = r.left + (LONG)rect.w;
        r.bottom = r.top + (LONG)rect.h;
        FillRect(hdc, &r, (HBRUSH)(COLOR_BTNFACE+1));
        DrawEdge(hdc, &r, EDGE_RAISED, BF_RECT);
    }
    void onClick(int x, int y) override {
        if (onClickHandler) onClickHandler();
    }
};

class Label : public Widget {
    std::wstring text;
public:
    Label(std::wstring t) : text(std::move(t)) {}
    void draw(HDC hdc, int ox = 0, int oy = 0) override {
        TextOutW(hdc, (int)(ox + rect.x), (int)(oy + rect.y), text.c_str(), (int)text.size());
        Widget::draw(hdc, ox, oy);
    }
};