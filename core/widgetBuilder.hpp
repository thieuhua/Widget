#pragma once
#include <memory>
#include <functional>
#include "widget.hpp"


class WidgetBuilder {
    std::unique_ptr<Widget> rootWidget;

public:
    WidgetBuilder() : rootWidget(std::make_unique<Widget>()) {}
    WidgetBuilder(std::unique_ptr<Widget> w) : rootWidget(std::move(w)) {}

    // ------------------------
    // set properties
    // ------------------------
    WidgetBuilder& size(double w, double h) {
        rootWidget->rect.w = w;
        rootWidget->rect.h = h;
        rootWidget->markLayoutDirty();
        return *this;
    }

    WidgetBuilder& position(double x, double y) {
        rootWidget->rect.x = x;
        rootWidget->rect.y = y;
        rootWidget->markLayoutDirty();
        return *this;
    }

    WidgetBuilder& visible(bool v) {
        rootWidget->setVisible(v);
        return *this;
    }

    WidgetBuilder& layout(std::unique_ptr<Layout> l) {
        rootWidget->layout = std::move(l);
        return *this;
    }

    template <typename LayoutT>
    requires std::is_base_of_v<Layout, LayoutT>
    WidgetBuilder& layout() {
        rootWidget->layout = std::make_unique<LayoutT>();
        return *this;
    }

    WidgetBuilder& onClick(std::function<void(Widget*)> cb) {
        struct CallbackWidget : public Widget {
            std::function<void(Widget*)> f;
            CallbackWidget(std::function<void(Widget*)> func) : f(func) {}
            void onClick(int, int) override { if(f) f(this); }
        };
        auto cbWidget = std::make_unique<CallbackWidget>(cb);
        cbWidget->rect = rootWidget->rect;
        rootWidget->addChild(std::move(cbWidget));
        return *this;
    }

    // ------------------------
    // add child builder
    // ------------------------
    WidgetBuilder& addChild(std::unique_ptr<Widget> child) {
        rootWidget->addChild(std::move(child));
        return *this;
    }

    WidgetBuilder& addChild(WidgetBuilder& childBuilder) {
        rootWidget->addChild(childBuilder.build());
        return *this;
    }
    // Accept builder by lvalue.
    WidgetBuilder& addChild(WidgetBuilder&& builder) {
        rootWidget->addChild(builder.build());
        return *this;
    }

    // ------------------------
    // get final widget
    // ------------------------
    std::unique_ptr<Widget> build() {
        if(!rootWidget) LOG("Warning: building a null widget");
        return std::move(rootWidget);
    }

    // ------------------------
    // helper static factory
    // ------------------------
    static WidgetBuilder create() { return WidgetBuilder(); }
    static WidgetBuilder create(std::unique_ptr<Widget> w) { return WidgetBuilder(std::move(w)); }
};

