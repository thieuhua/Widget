#pragma one
#include "../core/layout.hpp"
#include "../core/widget.hpp"


// DockLayout
enum class Dock { Left, Top, Right, Bottom, Fill };

class DockLayout : public Layout {
public:
    struct DockChild {
        Widget* widget;
        Dock dock;
    };
    std::vector<DockChild> dockChildren;


    

    void addDockedWidget(Widget* w, Dock d) {
        dockChildren.push_back({w,d});
    }

    Size measure(Widget* widget, const LayoutConstraints& c) override {
        // measure children independently; container prefers max of remaining space
        double totalW = 0, totalH = 0;
        for (auto &dc : dockChildren) {
            LayoutConstraints cc = c; // unconstrained
            Size s =dc.widget->measure(cc);
            totalW = std::max(totalW, s.w);
            totalH = std::max(totalH, s.h);
        }
        return { clampDouble(totalW, c.minW, c.maxW), clampDouble(totalH, c.minH, c.maxH) };
    }

    void arrange(Widget* widget, const Rect& bounds) override {
        Rect rem = {0, 0, bounds.w, bounds.h};

        for (auto &dc : dockChildren) {
            Widget* w = dc.widget;
            Rect r;
            Size pref = w->measure(LayoutConstraints::Unbounded());

            switch(dc.dock) {
                case Dock::Left:
                    r = { rem.x, rem.y, pref.w, rem.h };
                    rem.x += pref.w;
                    rem.w -= pref.w;
                    break;
                case Dock::Right:
                    r = { rem.x + rem.w - pref.w, rem.y, pref.w, rem.h };
                    rem.w -= pref.w;
                    break;
                case Dock::Top:
                    r = { rem.x, rem.y, rem.w, pref.h };
                    rem.y += pref.h;
                    rem.h -= pref.h;
                    break;
                case Dock::Bottom:
                    r = { rem.x, rem.y + rem.h - pref.h, rem.w, pref.h };
                    rem.h -= pref.h;
                    break;
                case Dock::Fill:
                    r = rem;
                    rem = {0,0,0,0}; // no remaining
                    break;
            }
            w->arrange(r);
        }
    }
};

#ifdef WIDGET_BUILDER_HPP

// class DockBuilder: public WidgetBuilder {
//     Dock DockDefault = Dock::Top;
//     using WidgetBuilder::WidgetBuilder;

//     WidgetBuilder& addChild(std::unique_ptr<Widget> child, Dock d = Dock::Top) {
//         __impl(child.get(), d);
//         rootWidget->addChild(std::move(child));
//         return *this;
//     }

//     WidgetBuilder& addChild(WidgetBuilder& childBuilder, Dock d = Dock::Top) {
//         rootWidget->addChild(childBuilder.build());
//         return *this;
//     }
//     // Accept builder by lvalue.
//     WidgetBuilder& addChild(WidgetBuilder&& builder) {
//         rootWidget->addChild(builder.build());
//         return *this;
//     }
// private:
//     void __impl(Widget* child, Dock d);
// };

// template <>
// auto WB<DockLayout>(){
//     return DockBuilder();
// }




#endif