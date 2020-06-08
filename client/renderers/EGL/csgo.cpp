#include "csgo.h"

#include "common/debug.h"
#include "utils.h"

extern "C" {
  #include "pipe.h"
}

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <cstring>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "imgui.h"

const char *esp_bodypart_list[] = {
  "Head",
  "Chest"
};

const char *status[] = {
  "Status: Cannot find csgo.exe",
  "Status: csgo.exe is running"
};

struct Csgo_glow
{
  bool enabled;
  float tv_color[4];
  float ti_color[4];
  float ctv_color[4];
  float cti_color[4];
};

struct Csgo_esp
{
  bool enabled;
  short bodypart_idx;
  float close_color[3];
  float distant_color[3];
};

struct Csgo_settings
{
  char name[32];
  short status_idx;
  bool enabled;

  struct Csgo_glow glow;
  struct Csgo_esp esp;
};

bool csgo_ovr_init(Csgo_settings ** csgo)
{
  *csgo = (Csgo_settings *)malloc(sizeof(Csgo_settings));
  if (!*csgo)
  {
    DEBUG_ERROR("Failed to malloc csgo");
    return false;
  }
  strncpy(
      (*csgo)->name,
      "CounterStrike: GO",
      strlen("CounterStrike: GO") + 1
  );
  (*csgo)->enabled = true;

  (*csgo)->glow.enabled = true;
  (*csgo)->glow.ctv_color[0] = 0.29803921569f;
  (*csgo)->glow.ctv_color[1] = 0.50196078431f;
  (*csgo)->glow.ctv_color[2] = 1.0f;
  (*csgo)->glow.ctv_color[3] = 1.0f;

  (*csgo)->glow.cti_color[0] = 0.09803921569f;
  (*csgo)->glow.cti_color[1] = 0.16078431373f;
  (*csgo)->glow.cti_color[2] = 0.32941176471f;
  (*csgo)->glow.cti_color[3] = 1.0f;

  (*csgo)->glow.tv_color[0] = 1.0f;
  (*csgo)->glow.tv_color[1] = 0.50196078431f;
  (*csgo)->glow.tv_color[2] = 0.29803921569f;
  (*csgo)->glow.tv_color[3] = 1.0f;

  (*csgo)->glow.ti_color[0] = 0.32941176471f;
  (*csgo)->glow.ti_color[1] = 0.16078431373f;
  (*csgo)->glow.ti_color[2] = 0.09803921569f;
  (*csgo)->glow.ti_color[3] = 1.0f;


  (*csgo)->esp.enabled = false;
  (*csgo)->esp.bodypart_idx = 0;
  (*csgo)->esp.enabled = false;

  return true;
}

void csgo_ovr_free(Csgo_settings ** csgo)
{
  if (!*csgo)
    return;

  free(*csgo);
  *csgo = NULL;
}

void csgo_ovr_render_menu(Csgo_settings * csgo, LG_Pipe * pipe)
{
  bool isChanged = false;
  if (ImGui::CollapsingHeader(csgo->name))
  {
    isChanged |= ImGui::Checkbox("Enabled", &csgo->enabled);
    ImGui::Text("%s", status[0]);
    if (ImGui::TreeNode("Options"))
    {
      if (ImGui::TreeNode("Glow"))
      {
        isChanged |= ImGui::Checkbox("Enabled", &csgo->glow.enabled);
        isChanged |= ImGui::ColorEdit3("CT visible  ", (float *)&csgo->glow.ctv_color, ImGuiColorEditFlags_NoInputs);
        ImGui::SameLine();
        isChanged |= ImGui::ColorEdit3("CT invisible", (float *)&csgo->glow.cti_color, ImGuiColorEditFlags_NoInputs);
        isChanged |= ImGui::ColorEdit3("T visible   ", (float *)&csgo->glow.tv_color, ImGuiColorEditFlags_NoInputs);
        ImGui::SameLine();
        isChanged |= ImGui::ColorEdit3("T invisible ", (float *)&csgo->glow.ti_color, ImGuiColorEditFlags_NoInputs);
        ImGui::TreePop();
      }
      if (ImGui::TreeNode("ESP"))
      {
        isChanged |= ImGui::Checkbox("Enabled", &csgo->esp.enabled);
        if (ImGui::BeginCombo(esp_bodypart_list[csgo->esp.bodypart_idx], esp_bodypart_list[csgo->esp.bodypart_idx], ImGuiComboFlags_NoPreview))
        {
          for (int n = 0; n < IM_ARRAYSIZE(esp_bodypart_list); n++)
          {
            const bool is_selected = (csgo->esp.bodypart_idx == n);
            if (ImGui::Selectable(esp_bodypart_list[n], is_selected))
            {
              csgo->esp.bodypart_idx = n;
              isChanged |= true;
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
              ImGui::SetItemDefaultFocus();
          }
          ImGui::EndCombo();
        }
        ImGui::SameLine();
        isChanged |= ImGui::ColorEdit3("Close  ", (float *)&csgo->esp.close_color, ImGuiColorEditFlags_NoInputs);
        ImGui::SameLine();
        isChanged |= ImGui::ColorEdit3("Distant", (float *)&csgo->esp.distant_color, ImGuiColorEditFlags_NoInputs);

        ImGui::TreePop();
      }
      ImGui::TreePop();
    }
  }
  if (isChanged)
  {
    pipe_send(pipe, csgo, sizeof(Csgo_settings));
  }
  return;
}

void csgo_ovr_render_overlay(Csgo_settings * csgo)
{
  if (!csgo->enabled)
    return;
}