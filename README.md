# WidgetLib

Thư viện GUI nhỏ bằng C++ cho Windows (sử dụng Win32 GDI). Mục tiêu của dự án là cung cấp các thành phần giao diện cơ bản (Widget, Layout, Panel, Label, Button, v.v.) để dễ dàng ghép thành giao diện 2D nhẹ, phục vụ thử nghiệm hoặc công cụ nội bộ.

## Tính năng chính
- Lớp cơ bản `Widget` với cây con (children) và các hook sự kiện (focus, click, v.v.)
- Hệ thống `Layout` linh hoạt: Stack, Grid, Dock, Flow, Absolute, ... (xem `default/layout.hpp`)
- Các panel mặc định: `Panel`, `StackPanel`, `GridPanel`, `ScrollPanel` (xem `default/Panel.hpp`)
- Một vài widget chuẩn: `Label`, `Button` (xem `default/label.hpp`, `default/base.hpp`)

> Lưu ý: hiện tại thư viện dùng trực tiếp các API Win32 (HDC, HFONT, RECT, v.v.), nên chỉ chạy trên Windows.

## Cấu trúc repository (tổng quan)

- `core/` — lõi của thư viện
	- `widget.hpp` — lớp `Widget`, duy trì cây widget, lifecycle, đo/arrange
	- `layout.hpp` — định nghĩa `Layout`, `LayoutConstraints`, `Size`, `Rect`, các interface đo/arrange
	- `widgetBuilder.hpp`, `widgetManager.hpp` — hỗ trợ tạo widget và layout chính
- `default/` — các widget và layout triển khai sẵn

- `README.md` — tài liệu (bạn đang đọc)

## Yêu cầu & Phụ thuộc
- Nền tảng: Windows (Win32 API)
- Compiler: bất kỳ trình biên dịch C++ tương thích (g++, clang++, MSVC) với thư viện GDI; ví dụ trên Windows với MinGW-w64 g++.

## Hướng dẫn build (ví dụ nhanh)

Thư viện hiện là tập hợp các header (.hpp). Để chạy một ví dụ, bạn cần 1 file `main.cpp` và link với `gdi32`/`user32`.

Ví dụ `main.cpp` (rất tối giản) — tạo cửa sổ Win32, khởi tạo widget và gọi draw (bạn cần triển khai vòng lặp Win32 và truyền HDC):

```cpp
#include "core/widget.hpp"
#include "default/Panel.hpp"
#include "default/label.hpp"

int main() {
		// Tạo window / HDC etc. (bạn cần thêm mã Win32 thông thường)
		// Đây chỉ ví dụ cấu trúc: include các header và dùng lớp
}
```

Biên dịch với MinGW (cmd.exe):

```bat
g++ -std=c++17 -I. main.cpp -o app.exe -lgdi32 -luser32
```

Ghi chú: nếu bạn sử dụng file nguồn phân tách (ví dụ `core/*.cpp`), hãy liệt kê tất cả file .cpp vào lệnh g++ hoặc dùng Makefile / VS Code tasks.

Trong workspace có sẵn một task VS Code: `build and debug` (nếu bạn mở folder trong VS Code, chạy Task -> Run Task -> "build and debug"). Task mặc định biên dịch file hiện tại bằng g++.

## Ví dụ dùng API (ý tưởng nhanh)

```cpp
// tạo panel dọc với label
auto root = std::make_unique<Widget>();
auto panel = std::make_unique<Panel>();
panel->setBackground(RGB(240,240,240));
auto lbl = std::make_unique<Label>(L"Xin chào");
panel->addChild(std::move(lbl));
root->addChild(std::move(panel));

// gọi measure/arrange từ widget manager hoặc trực tiếp
// root->measure(...); root->arrange(...);
```

## Ghi chú cho nhà phát triển
- Các file trong `default/` sử dụng `#include "../core/widget.hpp"` và `layout.hpp` — đảm bảo include path phù hợp khi biên dịch.
- Một số chức năng (ví dụ clone/layout cloning) có thể còn chưa hoàn chỉnh; xem các comment trong mã nguồn.

## Kiểm thử & phát triển tiếp
- Thêm một ứng dụng demo `examples/demo.cpp` sẽ giúp kiểm tra nhanh.
- Viết unit-test cho các hành vi layout (measure/arrange) nếu tách được phần logic khỏi GDI.

## Contributing
- Mọi PR/issue đều chào đón. Nếu mở PR lớn (refactor layout, API ổn định), vui lòng kèm tests và mẫu chạy.

## License

Dự án này tuân theo [MIT License](./LICENSE).
---

