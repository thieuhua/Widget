#include <memory>
// #include "widgetBuilder.hpp"

template <typename T>
class Wrapper: public T {
public:
    template<typename... Args>
    Wrapper(Args&&... args)
        : T(std::forward<Args>(args)...) {}
    Wrapper(Wrapper&&) = default;
};

// vi du trien khai cho Widget:
// class Widget
// {
// public:
//    Widget() = default;
//    Widget(Widget&& w) = default;
// };


// class Builder {
// public:
//     // some constructor

//     // hàm đa hình
//     static Builder create() {}
//     static Builder create(std::unique_ptr<Widget> widget);
    
//     template <typename WidgetT, typename... Args>
//     requires std::is_base_of_v<Widget, WidgetT>
//     static Builder create(Args&&... args) {
//         return WidgetBuilder(std::make_unique<WidgetT>(std::forward<Args>(args)...));
//     }
// };
// template <typename T = Widget, typename... Args>
// Builder WB(Args&&... args) {
//     if constexpr (std::is_same_v<T, Widget>) 
//         return Builder::create();
//     else
//     return Builder::create<T>(std::forward<Args>(args)...);
// }


// // khi muốn mở rộng:
// struct Dock {
//     int data;
// };

// template <typename T = Widget>
// requires std::is_base_of_v<Widget, T>
// class DockWidget: Wrapper<T> {
// public:
//     template<typename... Args>
//     DockWidget(Args&&... args)
//         : Wrapper<T>(std::forward<Args>(args)...), dock(d){}
//     DockWidget(Dock&& d): Wrapper<T>(), dock(dock) {}
//     Dock dock;
// };

// template<typename T = Widget>
// requires std::is_base_of_v<Widget, T>
// Builder WB(Dock&& d){
    
//     return Builder::create<DockWidget<T>>(std::move(d));
// }

// void do_something(){
//     Dock d;
//     auto root = WB(d);
// }