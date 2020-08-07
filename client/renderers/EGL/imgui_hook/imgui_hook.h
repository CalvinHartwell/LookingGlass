#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "cimgui.h"

void init_imgui(SDL_Window*);

ImDrawData* render_callback(SDL_Window*);
