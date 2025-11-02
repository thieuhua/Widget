#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <limits>

enum class Align { Start, Center, End, Stretch };
enum class Orientation { Vertical, Horizontal };

struct LayoutConstraints {
    double minW = 0;
    double minH = 0;
    double maxW = std::numeric_limits<double>::infinity();
    double maxH = std::numeric_limits<double>::infinity();

    static LayoutConstraints Unbounded() {
        return LayoutConstraints{};
    }
};
struct Rect {
    double x, y, w, h;
    bool contains(int px, int py) const {
        return px >= (int)x && px < (int)(x + w) && py >= (int)y && py < (int)(y + h);
    }
};


// small types
struct Size {
    double w = 0;
    double h = 0;
};

class Widget;

class Layout {
public:
    virtual ~Layout() = default;

    // measure widget (and its children) under given constraints -> preferred size
    virtual Size measure(Widget* widget, const LayoutConstraints& c) = 0;

    // arrange widget into given bounds (relative coordinates for widget->rect)
    virtual void arrange(Widget* widget, const Rect& bounds) = 0;
};

// -------------------------
// Utility: clamp helpers
// -------------------------
inline double clampDouble(double v, double lo, double hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

