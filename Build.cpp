#include "Ease.hpp"

EASE_WATCH_ME;

Build build(Flags flags) noexcept {
	flags.no_install_path = true;
	
	Build b = Build::get_default(flags);

	b.name = "Furvent";

	b.add_header("src/");
	b.add_source_recursively("src/");

	b.add_define("IMGUI_DISABLE_INCLUDE_IMCONFIG_H");

	b.add_library("opengl32");
	b.add_library("gdi32");
	b.add_library("shell32");
	b.add_library("user32");
	b.add_library("kernel32");
	b.add_library("vcruntime");
	b.add_library("msvcrt");
	b.add_library("lib/glfw3");
	// b.add_library("imgui");

	return b;
}