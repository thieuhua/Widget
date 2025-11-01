#pragma once
#include <windows.h>
#include <vector>
#include <memory>
#include <limits>
#include "layout.hpp"

#ifdef SET_DEBUG
    #include <iostream>
    #define LOG(s) std::cerr << s << std::endl
    #define LOG2(s, x)std::cerr << s<<":"<<x<<std::endl;
    #define DEBUG(var) LOG2(#var, var)
#else 
    #define LOG(s)
    #define LOG2(s, x)
    #define DEBUG(var)
#endif

// class Layout {
// public:
//     virtual ~Layout() = default;

//     // measure widget (and its children) under given constraints -> preferred size
//     virtual Size measure(Widget* widget, const LayoutConstraints& c) = 0;

//     // arrange widget into given bounds (absolute coordinates for widget->rect)
//     virtual void arrange(Widget* widget, const Rect& bounds) = 0;
// };


class WidgetBuilder;


class IEventWidget {
    public:
        // --- Mouse events ---
    virtual void onMouseEnter() {}
    virtual void onMouseLeave() {}
    virtual void onMouseMove(int x, int y) {}
    virtual void onMouseDown(int x, int y, int button) {}
    virtual void onMouseUp(int x, int y, int button) {}
    virtual void onClick(int x, int y) {}

    // --- Keyboard & focus ---
    virtual void onFocus() {}
    virtual void onBlur() {}
    virtual void onKeyDown(int key) {}
    virtual void onKeyUp(int key) {}
    virtual void onChar(wchar_t c) {}

    // --- Scroll support ---
    virtual void onScroll(int delta) {}
};



// -------------------------------
// Widget base class
class Widget : public IEventWidget {
    friend WidgetBuilder;
protected:
    // config
    bool visible = true;
    bool focusable = false;   // có thể nhận focus hay không
    // state
    bool dirty = true;
    bool layoutDirty = true;
    bool hovered = false;
    bool focused = false;


public:
    Rect rect;
    Widget* parent = nullptr;
    std::unique_ptr<Layout> layout;
    std::vector<std::unique_ptr<Widget>> children;

    Widget() = default;
    Widget(Widget&& other) = default;
    virtual ~Widget() = default;


    void setVisible(bool v) { visible = v; }
    void setFocusable(bool v) { focusable = v; }
    bool isFocusable() const { return focusable; }
    bool hasFocus() const { return focused; }

    // --- Drawing & Updating ---
    virtual void draw(HDC hdc, int ox=0, int oy=0) {
#ifdef SET_DEBUG
        // debug: draw border:
        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        Rectangle(hdc, ox + rect.x, oy + rect.y, 
                 ox + rect.x + rect.w, oy + rect.y + rect.h);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
#endif
        for (auto &c : children) c->draw(hdc, ox + (int)rect.x, oy + (int)rect.y);
    }

    virtual void update(double dt) {
        for (auto &c : children) c->update(dt);
    }

    // virtual overrides for events
    void onMouseEnter() { hovered = true; markDirty(); }
    void onMouseLeave() { hovered = false; markDirty(); }
    void onFocus() { focused = true; markDirty(); }
    void onBlur() { focused = false; markDirty(); }

    Size GetSize() {
        return {rect.w, rect.h};
    }
    virtual Size measure(const LayoutConstraints& c) const;
    virtual void arrange(const Rect& bounds);

    // --- Utility ---
    void addChild(std::unique_ptr<Widget> w) {
        w->parent = this;
        children.push_back(std::move(w));
        markDirty();
    }

    virtual void markDirty() {
        dirty = true;
        if (parent) parent->markDirty();
    }

    bool isDirty() const { return dirty; }
    void clearDirty() { dirty = false; for (auto &c : children) c->clearDirty(); }

    void markLayoutDirty() {
        layoutDirty = true;
        if (parent) parent->markLayoutDirty();
    }
    bool isLayoutDirty() const { return layoutDirty; }
    void clearLayoutDirty() { layoutDirty = false; for (auto &c : children) c->clearLayoutDirty(); }

    // hit test
    Widget* hitTest(int x, int y) {
        for (auto it = children.rbegin(); it != children.rend(); ++it) {
            Widget* child = it->get();
            int rx = x - (int)child->rect.x;
            int ry = y - (int)child->rect.y;
            if (child->rect.contains(rx, ry)) {
                Widget* hit = child->hitTest(rx, ry);
                return hit ? hit : child;
            }
        }
        if (rect.contains(x, y)) return this;
        return nullptr;
    }

    // find all focusable descendants
    void collectFocusable(std::vector<Widget*>& out) {
        if (focusable) out.push_back(this);
        for (auto& c : children) c->collectFocusable(out);
    }
    Widget clone();
    friend Widget Widget::clone();
};


Size Widget::measure(const LayoutConstraints& c) const{
    if (layout) {
        return layout->measure(const_cast<Widget*>(this), c);
    } else if (!children.empty()) {
        // container nhưng không có layout: chỉ cộng kích thước các children theo mặc định
        Size s{};
        for (const auto& child : children) {
            auto cs = child->measure(c);
            s.w = std::max(s.w, cs.w);
            s.h = std::max(s.h, cs.h);
        }
        return s;
    } else {
        // leaf widget: trả về size hiện tại hoặc mặc định
        return {rect.w > 0 ? rect.w : 1, rect.h > 0 ? rect.h : 1};
    }
}

void Widget::arrange(const Rect& bounds){
    DEBUG(bounds.x)
    DEBUG(bounds.y)
    rect = bounds;

    if (layout) {
        layout->arrange(this, bounds);
    } else {
        // container nhưng không có layout: có thể đặt children trùng với rect cha
        for (auto& child : children) {
            child->arrange(bounds); // mặc định full fill
        }
    }
}

Widget Widget::clone() {
    Widget copy;
    copy.rect = this->rect;
    copy.visible = this->visible;
    copy.focusable = this->focusable;
    // Note: layout cloning is not handled here; assuming shallow copy or null
    if (this->layout) {
        // If layout has a clone method, it should be called here
        // For now, we just set it to nullptr
        copy.layout = nullptr; 
    }
    for (const auto& child : this->children) {
        copy.children.push_back(std::make_unique<Widget>(child->clone()));
        copy.children.back()->parent = &copy;
    }
    return copy;
}