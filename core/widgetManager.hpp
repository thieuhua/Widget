#pragma once
#include "widget.hpp"
// #include "renderer.hpp"
#include <algorithm>

class Renderer;

class WidgetManager {
public:
    std::unique_ptr<Widget> root;
    Renderer* renderer;

    Widget* hoverTarget = nullptr;
    Widget* activeTarget = nullptr;
    Widget* focusTarget = nullptr;

    WidgetManager(std::unique_ptr<Widget> r, Renderer* ren) : root(std::move(r)), renderer(ren) {
        LOG("WidgetManager created");
        if(root) LOG("root widget exists");
    }

    ~WidgetManager() {
        LOG("WidgetManager destruction");
    }

    Size measure(const LayoutConstraints& c) {
        LOG("gdfdf");
        return root->measure(c);
    }

    void performLayout(LayoutConstraints rootConstraint) {
        Size desired = root->measure(rootConstraint);
        Rect rootRect { 0, 0, desired.w, desired.h };
        root->arrange(rootRect);
        root->clearLayoutDirty();
    }

    // ---------- Mouse ----------
    void onMouseMove(int x, int y) {
        Widget* hit = root->hitTest(x, y);
        if (hit != hoverTarget) {
            if (hoverTarget) hoverTarget->onMouseLeave();
            hoverTarget = hit;
            if (hoverTarget) hoverTarget->onMouseEnter();
        }
        if (hoverTarget) hoverTarget->onMouseMove(x, y);
    }

    void onMouseDown(int x, int y, int button) {
        LOG("WidgetManager onMouseDown at x:" << x << " y:" << y);
        Widget* hit = root->hitTest(x, y);
        if (hit) {
            Rect bound = hit->rect;
            LOG(" Hit widget at rect x:" << bound.x << " y:" << bound.y << " w:" << bound.w << " h:" << bound.h);
            activeTarget = hit;
            hit->onMouseDown(x, y, button);
            setFocus(hit);
        }
    }

    void onMouseUp(int x, int y, int button) {
        Widget* hit = root->hitTest(x, y);
        if (activeTarget) {
            activeTarget->onMouseUp(x, y, button);
            if (hit == activeTarget) hit->onClick(x, y);
            activeTarget = nullptr;
        }
    }

    void onScrollWheel(int delta) {
        // gửi sự kiện cho widget dưới chuột nếu có, hoặc widget focus
        if (hoverTarget) hoverTarget->onScroll(delta);
        else if (focusTarget) focusTarget->onScroll(delta);
    }

    // ---------- Focus management ----------
    void setFocus(Widget* w) {
        if (!w || !w->isFocusable()) return;
        if (focusTarget == w) return;
        if (focusTarget) focusTarget->onBlur();
        focusTarget = w;
        focusTarget->onFocus();
    }

    void clearFocus() {
        if (focusTarget) {
            focusTarget->onBlur();
            focusTarget = nullptr;
        }
    }

    void focusNext(bool reverse = false) {
        std::vector<Widget*> focusables;
        root->collectFocusable(focusables);
        if (focusables.empty()) return;

        auto it = std::find(focusables.begin(), focusables.end(), focusTarget);
        if (it == focusables.end()) {
            setFocus(focusables.front());
            return;
        }

        if (reverse) {
            if (it == focusables.begin()) it = focusables.end();
            --it;
        } else {
            ++it;
            if (it == focusables.end()) it = focusables.begin();
        }
        setFocus(*it);
    }

    // ---------- Keyboard ----------
    void onKeyDown(int key) {
        if (key == VK_TAB) { focusNext((GetKeyState(VK_SHIFT) & 0x8000) != 0); return; }
        if (key == VK_ESCAPE) clearFocus();
        if (focusTarget) focusTarget->onKeyDown(key);
    }

    void onKeyUp(int key) {
        if (focusTarget) focusTarget->onKeyUp(key);
    }

    void onChar(wchar_t c) {
        if (focusTarget) focusTarget->onChar(c);
    }

    // ---------- Frame tick ----------
    bool tick(double dt) {
        root->update(dt);
        return true;
    }
};
