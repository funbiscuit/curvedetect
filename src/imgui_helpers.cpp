#include "imgui_helpers.h"

#include "imgui.h"
#include "imgui_internal.h"

bool ImGui::IsMouseHoveringWindow()
{
    ImGuiContext& g = *GImGui;
    return g.HoveredWindow == g.CurrentWindow;
}