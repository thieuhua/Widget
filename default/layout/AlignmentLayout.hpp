#pragma one
#include "../core/layout.hpp"
#include "../core/widget.hpp"

// enum class Align { Start, Center, End, Stretch }; // defined in layout.hpps
struct Align2D {
    Align horizontal;
    Align vertical;

    constexpr Align2D(Align h, Align v) : horizontal(h), vertical(v) {}
};

// Toán tử "ghép" hai Align lại:
constexpr Align2D operator|(Align h, Align v) {
    return Align2D(h, v);
}



class AlignmentLayout : public Layout {

};

