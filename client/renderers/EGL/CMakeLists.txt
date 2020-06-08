cmake_minimum_required(VERSION 3.0)
project(renderer_EGL LANGUAGES C CXX)

find_package(PkgConfig)
pkg_check_modules(RENDERER_EGL_PKGCONFIG REQUIRED
	egl
	wayland-egl
	gl
)

include_directories(${PROJECT_TOP}/repos)
include_directories(${PROJECT_TOP}/repos/cimgui/generator/output)
include_directories(${PROJECT_TOP}/repos/cimgui/imgui)
include_directories(${PROJECT_TOP}/repos/cimgui/imgui/examples)
include_directories(${PROJECT_TOP}/repos/gl3w/build/include)

add_subdirectory(${PROJECT_TOP}/repos/cimgui "${CMAKE_BINARY_DIR}/cimgui")

add_library(imgui_impl STATIC 
	${PROJECT_TOP}/repos/gl3w/build/src/gl3w.c 
	${PROJECT_TOP}/repos/cimgui/imgui/examples/imgui_impl_opengl3.cpp
	${PROJECT_TOP}/repos/cimgui/imgui/examples/imgui_impl_sdl.cpp
)

target_compile_definitions(imgui_impl PRIVATE IMGUI_IMPL_API=extern\ \"C\")

target_link_libraries(imgui_impl
	cimgui
)
add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GL3W)


include(MakeObject)
make_object(
	EGL_SHADER
	shader/desktop.vert
	shader/desktop_rgb.frag
	shader/desktop_yuv.frag
	shader/cursor.vert
	shader/cursor_rgb.frag
	shader/cursor_mono.frag
	shader/fps.vert
	shader/fps.frag
	shader/fps_bg.frag
	shader/alert.vert
	shader/alert.frag
	shader/alert_bg.frag
	shader/splash_bg.vert
	shader/splash_bg.frag
	shader/splash_logo.vert
	shader/splash_logo.frag
)

add_library(renderer_EGL STATIC
	egl.c
	shader.c
	texture.c
	model.c
	desktop.c
	cursor.c
	fps.c
	draw.c
	splash.c
	alert.c
	overlay.c
	pipe.c
	csgo.cpp
	${EGL_SHADER_OBJS}
)

target_link_libraries(renderer_EGL
	${RENDERER_EGL_PKGCONFIG_LIBRARIES}
	lg_common
	fonts
	imgui_impl
	dl
)

target_include_directories(renderer_EGL
	PRIVATE
		src
		${EGL_SHADER_INCS}
		${RENDERER_EGL_PKGCONFIG_INCLUDE_DIRS}
)
