
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

void MainWindow::on_resize(int w, int h)
{
    width=w; height=h;
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
        controller.open_image();
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

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    //draw area
    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
    ImVec2 canvas_size = ImGui::GetContentRegionAvail();        // Resize canvas to what's available
    if (canvas_size.x < 50.0f) canvas_size.x = 50.0f;
    if (canvas_size.y < 50.0f) canvas_size.y = 50.0f;
    draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y),
                             IM_COL32(220, 220, 220, 255));
//    draw_list->AddRectFilledMultiColor(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y),
//            IM_COL32(50, 50, 50, 255), IM_COL32(50, 50, 60, 255), IM_COL32(60, 60, 70, 255), IM_COL32(50, 50, 60, 255));
    draw_list->AddRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y),
            IM_COL32(50, 50, 50, 255));

    ImGui::InvisibleButton("canvas", canvas_size);
//    ImVec2 mouse_pos_in_canvas = ImVec2(ImGui::GetIO().MousePos.x - canvas_pos.x, ImGui::GetIO().MousePos.y - canvas_pos.y);
//    if (adding_line)
//    {
//        adding_preview = true;
//        points.push_back(mouse_pos_in_canvas);
//        if (!ImGui::IsMouseDown(0))
//            adding_line = adding_preview = false;
//    }
    if (ImGui::IsItemHovered())
    {
//        if (!adding_line && ImGui::IsMouseClicked(0))
//        {
//            points.push_back(mouse_pos_in_canvas);
//            adding_line = true;
//        }
//        if (ImGui::IsMouseClicked(1) && !points.empty())
//        {
//            adding_line = adding_preview = false;
//            points.pop_back();
//            points.pop_back();
//        }
    }
//    draw_list->PushClipRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), true);      // clip lines within the canvas (if we resize it, etc.)
//    for (int i = 0; i < points.Size - 1; i += 2)
//        draw_list->AddLine(ImVec2(canvas_pos.x + points[i].x, canvas_pos.y + points[i].y), ImVec2(canvas_pos.x + points[i + 1].x, canvas_pos.y + points[i + 1].y), IM_COL32(255, 255, 0, 255), 2.0f);
//    draw_list->PopClipRect();




    if(controller.image)
    {
        auto tex_id = (ImTextureID)(intptr_t) controller.image->get_texture();

        float w = controller.image->get_width();
        float h = controller.image->get_height();

        auto sz=canvas_size;
        sz.x-=2;
        sz.y-=2;
        auto pos=canvas_pos;
        ++pos.x;
        ++pos.y;
        float availAspect=sz.x/sz.y;
        float aspect=w/h;
        if(aspect>availAspect)
        {
            w=sz.x;
            h=w/aspect;
            pos.y+=(sz.y-h)*0.5;
        }
        else
        {
            h=sz.y;
            w=h*aspect;
            pos.x+=(sz.x-w)*0.5;
        }

        ImGui::SetCursorScreenPos(pos);
        ImGui::Image(tex_id, ImVec2(w, h));
    }

}
