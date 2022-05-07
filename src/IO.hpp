#pragma once
#include <optional>
#include <vector>
#include <Windows.h>

struct Window {
	HWND handle;
};

struct Image {
	size_t width;
	size_t height;
	std::vector<std::uint8_t> pixels;
};

extern Window get_desktop_window() noexcept;
extern std::optional<Window> find_winamax_window() noexcept;
extern std::optional<Image> take_screenshot(Window window) noexcept;


