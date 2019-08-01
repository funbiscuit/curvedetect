#include "imgui_helpers.h"

#include "imgui.h"
#include "imgui_internal.h"

bool ImGui::is_mouse_hovering_window()
{
    ImGuiContext& g = *GImGui;
    return g.HoveredWindow == g.CurrentWindow;
}