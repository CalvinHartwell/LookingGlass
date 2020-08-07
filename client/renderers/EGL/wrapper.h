#include "/usr/include/SDL2/SDL.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "../../../repos/cimgui/cimgui.h"

#ifndef IMGUI_IMPL_API
#define IMGUI_IMPL_API
#endif

struct SDL_Window;
typedef union SDL_Event SDL_Event;

IMGUI_IMPL_API bool ImGui_ImplSDL2_InitForOpenGL(struct SDL_Window *window, void *sdl_gl_context);

IMGUI_IMPL_API bool ImGui_ImplSDL2_InitForVulkan(struct SDL_Window *window);

IMGUI_IMPL_API bool ImGui_ImplSDL2_InitForD3D(struct SDL_Window *window);

IMGUI_IMPL_API bool ImGui_ImplSDL2_InitForMetal(struct SDL_Window *window);

IMGUI_IMPL_API void ImGui_ImplSDL2_Shutdown();

IMGUI_IMPL_API void ImGui_ImplSDL2_NewFrame(struct SDL_Window *window);

IMGUI_IMPL_API bool ImGui_ImplSDL2_ProcessEvent(const SDL_Event *event);

// Backend API
IMGUI_IMPL_API bool ImGui_ImplOpenGL3_Init(const char *glsl_version);

IMGUI_IMPL_API void ImGui_ImplOpenGL3_Shutdown();

IMGUI_IMPL_API void ImGui_ImplOpenGL3_NewFrame();

IMGUI_IMPL_API void ImGui_ImplOpenGL3_RenderDrawData(ImDrawData *draw_data);

// (Optional) Called by Init/NewFrame/Shutdown
IMGUI_IMPL_API bool ImGui_ImplOpenGL3_CreateFontsTexture();

IMGUI_IMPL_API void ImGui_ImplOpenGL3_DestroyFontsTexture();

IMGUI_IMPL_API bool ImGui_ImplOpenGL3_CreateDeviceObjects();

IMGUI_IMPL_API void ImGui_ImplOpenGL3_DestroyDeviceObjects();
