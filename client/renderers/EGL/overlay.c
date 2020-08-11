#include "overlay.h"
#include "common/debug.h"
#include "utils.h"
#include "pipe.h"
#include "csgo.h"
#include "vect.h"

#include "./imgui_hook/imgui_hook.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include "cimgui.h"

#ifndef IMGUI_IMPL_API
#define IMGUI_IMPL_API
#endif

struct SDL_Window;
typedef union SDL_Event SDL_Event;

IMGUI_IMPL_API bool ImGui_ImplSDL2_InitForOpenGL(SDL_Window *window, void *sdl_gl_context);

IMGUI_IMPL_API bool ImGui_ImplSDL2_InitForVulkan(SDL_Window *window);

IMGUI_IMPL_API bool ImGui_ImplSDL2_InitForD3D(SDL_Window *window);

IMGUI_IMPL_API bool ImGui_ImplSDL2_InitForMetal(SDL_Window *window);

IMGUI_IMPL_API void ImGui_ImplSDL2_Shutdown();

IMGUI_IMPL_API void ImGui_ImplSDL2_NewFrame(SDL_Window *window);

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

struct EGL_Overlay {
    LG_Pipe *pipe;
    Csgo_settings *csgo;
    cvector_vector_type(LG_Text)primitives;
};

void egl_add_overlay_renderer(EGL_Overlay *imgui, bool *isEnabled) {
}

bool egl_overlay_init(EGL_Overlay **imgui, SDL_Window *w) {
    *imgui = (EGL_Overlay *) malloc(sizeof(EGL_Overlay));
    if (!*imgui) {
        DEBUG_ERROR("Failed to malloc EGL_Line");
        return false;
    }
    memset(*imgui, 0, sizeof(EGL_Overlay));

    start_pipe_thread();

    if (!csgo_ovr_init(&(*imgui)->csgo)) {
        DEBUG_ERROR("Failed to initialize csgo");
        return false;
    }

    init_imgui(w);
//    SDL_GLContext context = SDL_GL_CreateContext(w);
//    igCreateContext(NULL);
//    ImGui_ImplSDL2_InitForOpenGL(w, &context);
    ImGui_ImplOpenGL3_Init("#version 300 es");
//    igStyleColorsDark(NULL);

    return true;
}

void egl_overlay_free(EGL_Overlay **imgui) {
    if (!*imgui)
        return;

    cvector_free((*imgui)->primitives);
    free((*imgui)->csgo);
    free(*imgui);
    *imgui = NULL;
}

void egl_overlay_update(EGL_Overlay *imgui, void *data, size_t size) {
    for (size_t p = 0; p < size;) {
        char type = ((char *) data)[p];
        size_t sz = 0;
        switch (type) {
            case 0:
                sz += sizeof(LG_Null);
                break;
            case 1:
                sz += sizeof(LG_Line);
                break;
            case 2:
                sz += sizeof(LG_Box);
                break;
            case 3:
                sz += sizeof(LG_Text);
                break;
            default:
                DEBUG_WARN("Pipe data is corrupt this update - returning.");
                return;
        }

        LG_Text text;
        memcpy(&text, data + p, sz);
        if (text.idx >= cvector_size(imgui->primitives)) {
            cvector_push_back(imgui->primitives, text);
        } else {
            imgui->primitives[text.idx] = text;
        }
        p += sz;
    }
}

void egl_overlay_render(EGL_Overlay *imgui, SDL_Window *window, float scaleY) {
    ImGui_ImplOpenGL3_NewFrame();
    ImDrawData *data = render_callback(window);
    ImGui_ImplOpenGL3_RenderDrawData(data);
    SDL_GL_SwapWindow(window);
    return;
    static bool showMenu = false;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    igNewFrame();

    igRender();
    igEndFrame();

    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
    SDL_GL_SwapWindow(window);

    if (igIsKeyPressed(SDL_SCANCODE_NUMLOCKCLEAR, false))
        showMenu = !showMenu;

    if (showMenu) {
        igShowDemoWindow(NULL);
        if (igBegin("Menu", NULL, ImGuiWindowFlags_None)) {
            csgo_ovr_render_menu(imgui->csgo, imgui->pipe);
        }
        igEnd();
    } else {
        igSetMouseCursor(ImGuiMouseCursor_None);
    }

    ImVec2 zero = {0, 0};
    igSetNextWindowPos(zero, 0, zero);
    igSetNextWindowSize(igGetIO()->DisplaySize, 0);
    if (igBegin("Overlay", NULL, ImGuiWindowFlags_NoTitleBar
                                 | ImGuiWindowFlags_NoResize
                                 | ImGuiWindowFlags_NoMove
                                 | ImGuiWindowFlags_NoScrollbar
                                 | ImGuiWindowFlags_NoSavedSettings
                                 | ImGuiWindowFlags_NoInputs
                                 | ImGuiWindowFlags_NoBackground)) {
        ImDrawList *dl = igGetWindowDrawList();
        for (size_t p = 0; p < cvector_size(imgui->primitives); ++p) {
            char type = imgui->primitives[p].type;
            switch (type) {
                case 1: {
                    LG_Line *linePtr = (LG_Line *) &imgui->primitives[p];
                    ImVec2 p1 = {linePtr->x1, linePtr->y1};
                    ImVec2 p2 = {linePtr->x2, linePtr->y2};
                    ImDrawList_AddLine(dl, p1, p2, linePtr->color, linePtr->width);
                    break;
                }
                case 2: {
                    LG_Box *boxPtr = (LG_Box *) &imgui->primitives[p];
                    ImVec2 p1 = {boxPtr->x1, boxPtr->y1};
                    ImVec2 p2 = {boxPtr->x2, boxPtr->y2};
                    if (boxPtr->filled) {
                        ImDrawList_AddRectFilled(dl, p1, p2, boxPtr->color, 0.0f, ImDrawCornerFlags_None);
                    } else {
                        ImDrawList_AddRect(dl, p1, p2, boxPtr->color, 0.0f, ImDrawCornerFlags_None, boxPtr->thickness);
                    }
                    break;
                }
                case 3: {
                    LG_Text *textPtr = (LG_Text *) &imgui->primitives[p];
                    ImVec2 p1 = {textPtr->x, textPtr->y};
                    ImDrawList_AddTextVec2(dl, p1, textPtr->color, textPtr->str, NULL);
                    break;
                }
            }
        }
        igEnd();
    }

    igRender();
    igEndFrame();

    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
    SDL_GL_SwapWindow(window);
}