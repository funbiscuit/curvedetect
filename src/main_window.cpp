
#include "main_window.h"
#include "imgui.h"


MainWindow::MainWindow()
{
    width=1024;
    height=720;
    toolbar_width=280;
}


void MainWindow::on_render()
{

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
//    if (no_scrollbar)       window_flags |= ImGuiWindowFlags_NoScrollbar;
//    if (!no_menu)           window_flags |= ImGuiWindowFlags_MenuBar;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
//    if (no_nav)             window_flags |= ImGuiWindowFlags_NoNav;
    window_flags |= ImGuiWindowFlags_NoBackground;
//    if (no_bring_to_front)  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(toolbar_width, height));
    ImGui::Begin("Main", nullptr, window_flags);
    render_toolbar();
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(toolbar_width, 0));
    ImGui::SetNextWindowSize(ImVec2(width-toolbar_width, height));
    ImGui::Begin("Area", nullptr, window_flags);
    render_area();
    ImGui::End();

}

void MainWindow::render_toolbar()
{
    static int subdiv = 3;
    static int bin_thresh = 127;
    static bool show_subdiv = true;
    static bool show_image = true;
    static bool show_bin = false;

    ImGui::Text("Subdivision: %d", subdiv);
    ImGui::PushItemWidth(-1);
    ImGui::SliderInt("##subdiv", &subdiv, 0, 8);
    ImGui::PopItemWidth();


    ImGui::Text("Bin threshold: %d", bin_thresh);
    ImGui::PushItemWidth(-1);
    ImGui::SliderInt("##bin-thresh", &bin_thresh, 0, 255);
    ImGui::PopItemWidth();

    ImGui::Checkbox("Show subdivision", &show_subdiv);
    ImGui::Checkbox("Show image", &show_image);
    ImGui::Checkbox("Show binarization", &show_bin);


    if (ImGui::Button("Open"))
    {

    }
    ImGui::SameLine();
    if (ImGui::Button("Reset"))
    {

    }
    ImGui::Separator();

    if (ImGui::Button("Copy"))
    {

    }
    ImGui::SameLine();
    if (ImGui::Button("Export"))
    {

    }

    ImGui::Text("Column separator:");
    ImGui::SameLine(190);
    const char* col_sep_items[] = { "\\t", " ", ",", ";" };
    static int col_sep = 0;
    ImGui::PushItemWidth(80);
    ImGui::Combo("##col-sep", &col_sep, col_sep_items, IM_ARRAYSIZE(col_sep_items));
    ImGui::PopItemWidth();


    ImGui::Text("Line ending:");
    ImGui::SameLine(190);
    const char* line_end_items[] = { "\\n", ";", "," };
    static int line_end = 0;
    ImGui::PushItemWidth(80);
    ImGui::Combo("##line-end", &line_end, line_end_items, IM_ARRAYSIZE(line_end_items));
    ImGui::PopItemWidth();


    ImGui::Text("Decimal separator:");
    ImGui::SameLine(190);
    const char* dec_sep_items[] = { ".", "," };
    static int dec_sep = 0;
    ImGui::PushItemWidth(80);
    ImGui::Combo("##dec-sep", &dec_sep, dec_sep_items, IM_ARRAYSIZE(dec_sep_items));
    ImGui::PopItemWidth();

    ImGui::Separator();
    render_hints();
}

void MainWindow::render_hints()
{
    ImGui::Text("Open new image");
}

void MainWindow::render_area()
{
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);


}
