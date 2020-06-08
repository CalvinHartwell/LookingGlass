
#pragma once

#include <stdbool.h>

typedef struct Csgo_settings Csgo_settings;

// forward
typedef struct LG_Pipe LG_Pipe;

#ifdef __cplusplus
extern "C" {
#endif
  bool csgo_ovr_init(Csgo_settings ** csgo);
  void csgo_ovr_free(Csgo_settings **csgo);
  void csgo_ovr_render_menu(Csgo_settings * csgo, LG_Pipe * pipe);
  void csgo_ovr_render_overlay(Csgo_settings * csgo);
#ifdef __cplusplus
}
#endif