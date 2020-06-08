#pragma once

#include <stdbool.h>

#include "interface/renderer.h"
#include "interface/font.h"

#include <SDL2/SDL_egl.h>

#if defined(SDL_VIDEO_DRIVER_WAYLAND)
#include <wayland-egl.h>
#endif

typedef struct EGL_Overlay EGL_Overlay;

bool egl_overlay_init(EGL_Overlay ** imgui, SDL_Window * window);
void egl_overlay_free(EGL_Overlay ** imgui);

void egl_overlay_update(EGL_Overlay * imgui, void * data, size_t size);
void egl_overlay_render(EGL_Overlay * imgui, SDL_Window * window, float scaleY);
