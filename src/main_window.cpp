
#include <iostream>
#include <string>
#include <vector>

#include "main_window.h"
#include "main_app.h"
#include "clipboard.h"
#include "imgui.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_helpers.h"
#include "glad/glad.h"

#include <GLFW/glfw3.h>
#include <cmath>
#include <fstream>

#include "tinyfiledialogs.h"

MainWindow::MainWindow()
{
    width=1024;
    height=720;
    toolbar_width=280;

    currentMode = MODE_POINTS;


    minImageScale = 1.0f;
    imageScale = 1.0f;
    maxImageScale = 3.0f;

    zoomPixelHalfSide = 30;

    zoomWindowSize = 200.0f;

    decimalSeparator = '.';
    columnSeparator = "\t";
    lineEnding = "\n";

    image = nullptr;
    curve = nullptr;

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
    window_flags |= ImGuiWindowFlags_NoSavedSettings;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;

//    ImGui::SetNextWindowPos(ImVec2(0, 0));
//    ImGui::SetNextWindowSize(ImVec2(toolbar_width, height));
//    ImGui::Begin("Main", nullptr, window_flags);
//    render_toolbar();
//    ImGui::End();
//
//    ImGui::SetNextWindowPos(ImVec2(toolbar_width, 0));
//    ImGui::SetNextWindowSize(ImVec2(width-toolbar_width, height));
//    ImGui::Begin("Area", nullptr, window_flags);
//    render_area();
//    ImGui::End();


    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)width, (float)height));// , ImGuiSetCond_FirstUseEver);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    if (ImGui::Begin("Main Window", nullptr, window_flags))
    {
        render_main_window();
        ImGui::End();
    }
    ImGui::PopStyleVar();

}

void MainWindow::init()
{
#ifndef NDEBUG
    image=std::make_shared<Image>("../img/test.png");
    curve=std::make_shared<CurveDetect>(image);
    curve->reset_all();
#endif
}

void MainWindow::on_resize(int w, int h)
{
    width=w; height=h;
}

void MainWindow::render_main_window()
{
    if(bShowFps)
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    render_side_panel();

    ImGui::SameLine();


    ImGui::BeginGroup();


    // Create our canvas

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImVec4(30, 30, 30, 255));
    ImGui::BeginChild("image_region", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);

    ImVec2 canvas_sz = ImGui::GetWindowSize();

    render_image(canvas_sz);

    ImVec2 WinPos = ImGui::GetWindowPos();
    ImVec2 MousePos = ImGui::GetMousePos();
    hoveredImagePixel = (MousePos - WinPos - imagePosition)/imageScale;

    if(curve)
        curve->update_hovered(Vec2D(hoveredImagePixel));

    process_input();


    render_horizon(imagePosition);
    render_grid_lines(imagePosition);
    render_points(imageScale, imagePosition, MousePos);

    ImVec2 ZoomOrigin;

    if (image && bShowZoomWindow && render_zoom_window(canvas_sz, ZoomOrigin))
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        float zoomedScale = zoomWindowSize/ float(2 * zoomPixelHalfSide + 1);

        ImVec2 zoomedImPos = ZoomOrigin -hoveredImagePixel*zoomedScale-WinPos;

        ImVec2 ZoomWnd = ImVec2(zoomWindowSize, zoomWindowSize);

        //TODO add ticks and horizon
        //draw zoomed curve in zoom window
        draw_list->PushClipRect(ZoomOrigin- ZoomWnd*0.5f, ZoomOrigin+ ZoomWnd*0.5f, true);
        render_points(zoomedScale, zoomedImPos, ZoomOrigin);
        draw_list->PopClipRect();

    }




    // Draw context menu
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
    if (ImGui::BeginPopup("context_menu"))
    {
        bIsContextMenuOpened = true;
        bIsReadyForAction = false;

        const char* items[]={"[1] Points","[2] Grid",
                             "[3] Horizon"};
        ActionMode modes[]={MODE_POINTS,MODE_GRID,MODE_HORIZON};

        for(int j=0;j<3;++j)
        {
            if (ImGui::MenuItem(items[j], nullptr, currentMode == modes[j], true))
            {
                currentMode = modes[j];
                bIsReadyForAction = true;
            }

        }

        ImGui::EndPopup();
    }
    else
    {
        bIsContextMenuOpened = false;
    }
    ImGui::PopStyleVar();


    render_tick_config_popup();



    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
    ImGui::EndGroup();
}

void MainWindow::render_image(ImVec2 canvasSize)
{
    if(image)
    {
        float im_width = image->get_width();
        float im_height = image->get_height();

        GLuint texID = image->get_texture();

        float scaleX, scaleY;

        scaleX = canvasSize.x / im_width;
        scaleY = canvasSize.y / im_height;

        minImageScale = scaleX < scaleY ? scaleX : scaleY;

        bool bScaleChanged = false;

        if (ImGui::is_mouse_hovering_window())
        {
            ImGuiIO& imIO = ImGui::GetIO();

            if (imIO.MouseWheel != 0)
            {
                imageScale *= std::pow(1.1f, imIO.MouseWheel);
                bScaleChanged = true;
            }
        }

        if (imageScale < minImageScale)
            imageScale = minImageScale;
        if (imageScale > maxImageScale)
            imageScale = maxImageScale;

        ImVec2 WinPos = ImGui::GetWindowPos();
        ImVec2 MousePos = ImGui::GetMousePos();
        if(bScaleChanged)
            imagePosition = MousePos - hoveredImagePixel*imageScale - WinPos;

        ImVec2 MinImPos, MaxImPos;
        MinImPos.x = canvasSize.x - im_width*imageScale;
        MinImPos.y = canvasSize.y - im_height*imageScale;

        MaxImPos = ImVec2(0.0f, 0.0f);

        if (MinImPos.x > MaxImPos.x)
        {
            MinImPos.x /= 2.0f;
            MaxImPos.x = MinImPos.x;
        }
        if (MinImPos.y > MaxImPos.y)
        {
            MinImPos.y /= 2.0f;
            MaxImPos.y = MinImPos.y;
        }

        if(ImGui::IsMouseDragging(2))
        {
            imagePosition += ImGui::GetMouseDragDelta(2);
            ImGui::ResetMouseDragDelta(2);
        }

        imagePosition.x = imagePosition.x < MinImPos.x ? MinImPos.x : (imagePosition.x > MaxImPos.x ? MaxImPos.x : imagePosition.x);
        imagePosition.y = imagePosition.y < MinImPos.y ? MinImPos.y : (imagePosition.y > MaxImPos.y ? MaxImPos.y : imagePosition.y);

        im_width*=imageScale;
        im_height*=imageScale;

        if (bShowImage)
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 cur_pos = WinPos+imagePosition;
            draw_list->AddImage((void *)(intptr_t)(texID), cur_pos, cur_pos + ImVec2(im_width, im_height));
        }

        //this is used to distinguish in shader
        //between our image (that should be binarized)
        //and all other textures (that should be rendered without changes)
        ImGui_ImplOpenGL3_SetImageBin(texID, bShowBinarization);
    }
}

void MainWindow::on_mouse_down(int btn)
{
    auto& app= MainApp::get();
    if (btn == 0)
    {
        switch (currentMode)
        {
            case MODE_POINTS:
                if (app.is_ctrl_down())
                    curve->add_point(Vec2D(hoveredImagePixel));
                else
                    curve->select_hovered(ImageElement::POINT);
                break;
            case MODE_HORIZON:
                curve->select_hovered(ImageElement::HORIZON);
                break;
            case MODE_GRID:
                if (curve->select_hovered(ImageElement::TICKS))
                    curve->backup_selected_tick();
                break;
            default:
                break;
        }
    }
}

void MainWindow::on_mouse_up(int btn)
{
    auto& app= MainApp::get();
    if (btn == 0)
    {
        switch (currentMode)
        {
            case MODE_POINTS:
            case MODE_HORIZON:
                if (deleteOnRelease)
                    curve->delete_selected();
                else
                    curve->deselect_all();
                break;
            case MODE_GRID:
                curve->deselect_all();
                break;
            default:
                break;
        }
    }
}

void MainWindow::on_mouse_double_click(int btn)
{
    auto& app= MainApp::get();
    if (btn == 0)
    {
        switch (currentMode)
        {
            case MODE_GRID:
                if(curve->get_selected_id())
                    ImGui::OpenPopup("TickConfig");
                break;
            default:
                break;
        }
    }
}

void MainWindow::on_mouse_drag(int btn)
{
    auto& app= MainApp::get();
    if (btn == 0 && curve)
    {
        deleteOnRelease = false;

        if(curve->move_selected(Vec2D(hoveredImagePixel)))
        {
            if(app.is_ctrl_down())
                curve->snap_selected();

            if(currentMode == MODE_POINTS || currentMode == MODE_HORIZON)
            {
                curve->update_subdiv();
            }
        }
    }
}

void MainWindow::process_input()
{
    auto& app= MainApp::get();
    static bool bIsMouseDownFirst = true;
    static ImVec2 lastMousePos = ImGui::GetMousePos();
    static bool prevCtrl = app.is_ctrl_down();
    ImVec2 delta = ImGui::GetMousePos()-lastMousePos;
    bool moving = (std::abs(delta.x)+std::abs(delta.y))>0.f || prevCtrl != app.is_ctrl_down();


    if (ImGui::IsWindowFocused())
    {
        ImGuiIO& io = ImGui::GetIO();
        ActionMode modes[]={MODE_POINTS,MODE_GRID,MODE_HORIZON};

        for(int j=0;j<3;++j)
            if (io.KeysDown[GLFW_KEY_1+j] || io.KeysDown[GLFW_KEY_KP_1+j])
                currentMode = modes[j];

        if(io.KeysDown[GLFW_KEY_F] && io.KeysDownDurationPrev[GLFW_KEY_F]==0.f)
            bShowFps = !bShowFps;

        if(curve && curve->get_selected_id()==0)
            deleteOnRelease = app.is_shift_down();
    }

    if (curve && ImGui::is_mouse_hovering_window() && !bIsContextMenuOpened && bIsReadyForAction)
    {
        if (ImGui::IsMouseDown(0) && image->is_pixel_inside((int) hoveredImagePixel.x, (int) hoveredImagePixel.y))
        {
            if (bIsMouseDownFirst) //first press
            {
                on_mouse_down(0);
            }
            else if(moving)
            {
                on_mouse_drag(0);
            }
            bIsMouseDownFirst = false;
        }
        if (ImGui::IsMouseReleased(0))
        {
            on_mouse_up(0);

            bIsMouseDownFirst = true;
        }
        if(ImGui::IsMouseDoubleClicked(0))
        {
            on_mouse_double_click(0);
            bIsMouseDownFirst = true;
        }

        // Open context menu
        if (ImGui::IsMouseClicked(1))
        {
            ImGui::OpenPopup("context_menu");
        }

    }

    if(curve)
        curve->check_horizon();

    if (ImGui::IsMouseReleased(0) && !bIsContextMenuOpened)
    {
        bIsReadyForAction = true;
    }
    lastMousePos+=delta;
    prevCtrl = app.is_ctrl_down();
}

void MainWindow::render_points(float ImageScale, ImVec2 im_pos, ImVec2 MousePos)
{
    if(!curve)
        return;

    ImVec2 WinPos = ImGui::GetWindowPos();


    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    uint64_t selectedId= curve->get_selected_id();
    uint64_t hoveredId= curve->get_hovered_id(ImageElement::POINT);
    auto& segments= curve->get_segments();
    auto& userPoints= curve->get_user_points();

    float subdivSize = 8.f;
    float userSize = 12.f;
    auto lineColor = ImColor(128, 128, 128, 255);
    auto pointStroke = ImColor(64, 64, 64, 255);
    auto userHover = ImColor(255, 255, 255, 255);
    auto userFill = ImColor(124, 252, 0, 255);
    auto deleteFill = ImColor(205, 92, 92, 255);
    auto subdivFill = ImColor(128, 128, 128, 255);
    auto subdivBadFill = deleteFill;

    double subdivSpacing = subdivSize;

    //draw lines between subdivided points
    if (!segments.empty())
    {
        for(auto& segment : segments)
        {
            ImVec2 PointPos0 = segment.begin.imagePosition.to_imvec() * ImageScale + im_pos + WinPos;

            for(int i=1;i<segment.points.size();++i)
            {
                const ImagePoint& point = segment.points[i];
                ImVec2 PointPos1 = point.imagePosition.to_imvec() * ImageScale + im_pos + WinPos;

                draw_list->AddLine(PointPos0, PointPos1, lineColor, 1.5f);
                PointPos0=PointPos1;
            }
        }

        //used to limit number of drawn points (so points are not drawn on top of each other)
        ImVec2 LastDrawnPos = segments[0].begin.imagePosition.to_imvec() * ImageScale + im_pos + WinPos;
        if (bShowSubdivPoints)
        {
            for (auto &segment : segments) {
                for (int i = 1; i < segment.points.size(); ++i) {
                    const ImagePoint &point = segment.points[i];
                    ImVec2 PointPos1 = point.imagePosition.to_imvec() * ImageScale + im_pos + WinPos;

                    ImU32 fill = point.isSnapped ? subdivFill : subdivBadFill;

                    double dist1 = Vec2D(PointPos1 - LastDrawnPos).norm2();

                    if (point.isSubdivisionPoint && (dist1 > subdivSpacing * subdivSpacing))
                    {
                        draw_list->AddCircleFilled(PointPos1, subdivSize * 0.5f, fill);
                        draw_list->AddCircle(PointPos1, subdivSize * 0.5f, pointStroke);
                        LastDrawnPos = PointPos1;
                    }
                }

            }
        }
    }


    //TODO maybe not needed
    //if we are pressing button - draw a line from press location to snapped point
//    if (SelectedItem >= 0 && ImGui::IsMouseDown(0) && CurrentMode == ActionMode1_AddPoints
//        && SelectedItem < int(userPoints.size()) && ImGui::IsMouseHoveringWindow())
//    {
//        ImVec2 PointPos0 = userPoints[SelectedItem] * im_scale + im_pos + WinPos;
//
//        ImU32 LineColor = ImColor(80, 220, 80, 220);
//        draw_list->AddLine(PointPos0, MousePos, LineColor, 2.0f);
//    }

    auto& app= MainApp::get();
    for (const auto &point : userPoints) {
        ImVec2 PointPos = point.imagePosition.to_imvec() * ImageScale + im_pos + WinPos;

        ImU32 fill = userFill;

        if (currentMode == MODE_POINTS)
            if (point.id == selectedId || (selectedId == 0 && point.id == hoveredId))
                fill = deleteOnRelease ? deleteFill : userHover;

        draw_list->AddCircleFilled(PointPos, userSize*0.5f, fill);
        draw_list->AddCircle(PointPos, userSize*0.5f, pointStroke);
    }
}

int onTickInput(ImGuiInputTextCallbackData *data)
{
    std::string tickVal = data->Buf;
    ImageTickLine::filter_value(tickVal, false);
    strcpy(data->Buf,tickVal.c_str());
    data->BufTextLen=tickVal.length();
    if(data->CursorPos>tickVal.length())
        data->CursorPos=tickVal.length();
    data->BufDirty=true;
    return 0;
}

void MainWindow::render_tick_config_popup()
{
    if(!curve)
        return;

    static bool bTickInputAutoFocus = true;

    static bool bTickConfigInit = true;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
    if (ImGui::BeginPopupModal("TickConfig", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {

        if (ImGui::IsMouseDown(0))
        {
            bTickInputAutoFocus = false;
        }

        //ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));
        ImGui::Text("Enter value for this tick line");
        ImGui::Separator();

        static std::string tickVal = "0";
        char buf[65];

        auto selected=(ImageTickLine*) curve->get_selected();
        auto& XTicks= curve->get_xticks();
        auto& YTicks= curve->get_yticks();

        if (bTickConfigInit && selected)
        {
            tickVal = selected->tickValueStr;
            ImageTickLine::filter_value(tickVal, false);

            bTickConfigInit = false;
        }

        //ImGui::RadioButton("X (vertical)", &TickType, 0);
        //ImGui::RadioButton("Y (horizontal)", &TickType, 1);
        //if (bTickInputAutoFocus)//set focus to float only when we just opened the window
        //	ImGui::SetKeyboardFocusHere(0);

        if (!ImGui::IsAnyItemActive())
            ImGui::SetKeyboardFocusHere();

        strcpy(buf,tickVal.c_str());
        ImGui::InputText("Value", buf, 30, ImGuiInputTextFlags_CallbackAlways, &onTickInput);
        tickVal=buf;

        auto& app= MainApp::get();

        if (ImGui::Button("OK", ImVec2(120, 0)) || app.is_enter_up())
        {
            ImageTickLine::filter_value(tickVal, true);

            if(selected)
                selected->set_value(tickVal.c_str());

            curve->deselect_all();

            bTickConfigInit = true;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {

            if(selected)
                selected->restore_backup();

            curve->deselect_all();

            bTickConfigInit = true;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();


    }
    else
    {
        bTickInputAutoFocus = true;
    }
    ImGui::PopStyleVar(2);
}

bool MainWindow::render_zoom_window(const ImVec2 &canvas_sz, ImVec2 &out_ZoomOrigin)
{
    if(!image)
        return false;

    ImVec2 WinPos = ImGui::GetWindowPos();
    ImVec2 MousePos = ImGui::GetMousePos();

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    float im_width = float(image->get_width())*imageScale;
    float im_height = float(image->get_height())*imageScale;

    float im_aspect = im_height / im_width;


    ImVec2 ZoomUV0, ZoomUV1, ZoomPos, ZoomWndOffset;
    float ZoomRectBorder = 5.0f;

    ZoomWndOffset = ImVec2(15.f, 15.f);
    ZoomPos = MousePos+ZoomWndOffset;

    if (ZoomPos.x + zoomWindowSize + ZoomRectBorder * 2 - WinPos.x > canvas_sz.x)
        ZoomPos.x = MousePos.x - (zoomWindowSize + ZoomRectBorder * 2);

    if (ZoomPos.y + zoomWindowSize + ZoomRectBorder * 2 - WinPos.y > canvas_sz.y)
        ZoomPos.y = MousePos.y - (zoomWindowSize + ZoomRectBorder * 2);

    if (ZoomPos.x - ZoomRectBorder * 2 - WinPos.x < 0)
        ZoomPos.x = ZoomRectBorder * 2 + WinPos.x;

    if (ZoomPos.y - ZoomRectBorder * 2 - WinPos.y < 0)
        ZoomPos.y = ZoomRectBorder * 2 + WinPos.y;

    ZoomUV0 = hoveredImagePixel;
    ZoomUV0.x /= image->get_width();
    ZoomUV0.y /= image->get_height();

    out_ZoomOrigin = ZoomPos + ImVec2(zoomWindowSize*0.5f,zoomWindowSize*0.5f);

    float ZoomUVsideU = float(2 * zoomPixelHalfSide + 1) / float(image->get_width());


    ZoomUV0 -= ImVec2(ZoomUVsideU, ZoomUVsideU / im_aspect)*0.5f;
    ZoomUV1 = ZoomUV0 + ImVec2(ZoomUVsideU, ZoomUVsideU / im_aspect);

    //draw zoom window
    if (ImGui::is_mouse_hovering_window())
    {
        ImU32 ZoomBgColor = ImColor(120, 120, 120, 220);
        draw_list->AddRectFilled(ZoomPos - ImVec2(ZoomRectBorder, ZoomRectBorder),
                                 ZoomPos + ImVec2(ZoomRectBorder+ zoomWindowSize, ZoomRectBorder+ zoomWindowSize),
                                 ZoomBgColor, 5.0f);

        GLuint texID = image->get_texture();
        draw_list->AddImage((void *)(intptr_t)(texID), ZoomPos,
                            ZoomPos + ImVec2(zoomWindowSize,zoomWindowSize), ZoomUV0, ZoomUV1);

        ImU32 CrossColor = ImColor(120, 120, 120, 220);
        draw_list->AddLine(ImVec2(zoomWindowSize*0.5f, 0.f) + ZoomPos,
                           ImVec2(zoomWindowSize*0.5f, zoomWindowSize) + ZoomPos, CrossColor);
        draw_list->AddLine(ImVec2(0.f, zoomWindowSize*0.5f) + ZoomPos,
                           ImVec2(zoomWindowSize, zoomWindowSize*0.5f) + ZoomPos, CrossColor);

        return true;
    }

    return false;
}

void MainWindow::render_grid_lines(ImVec2 im_pos)
{
    if(!curve)
        return;

    ImVec2 WinPos = ImGui::GetWindowPos();


    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    const auto tickColor = ImColor(128, 128, 128, 255);
    const auto tickHover = ImColor(152, 248, 59, 255);
    const auto tickSel = ImColor(59, 155, 59, 255);


    auto horizon= curve->get_horizon();
    auto CoordOriginImg= horizon.imagePosition.to_imvec();
    auto CoordOriginTargetX= horizon.target.imagePosition.to_imvec();
    uint64_t hoveredId= curve->get_hovered_id(ImageElement::TICKS);
    uint64_t selectedId= curve->get_selected_id();
    auto& XTicks= curve->get_xticks();
    auto& YTicks= curve->get_yticks();

    ImVec2 TargetDirX = CoordOriginTargetX - CoordOriginImg;

    ImVec2 TargetDirY;
    TargetDirY.x = TargetDirX.y;
    TargetDirY.y = -TargetDirX.x;

    ImVec2 LineStart, LineEnd, LabelPos;

    ImVec2 LineMargin = ImVec2(10.0f, 10.0f);

    //draw tick lines
    for (auto &tick : XTicks) {
        ImU32 col = tickColor;

        if (currentMode & MODE_GRID)
        {
            if (tick.id == selectedId)
                col = tickSel;
            else if (tick.id == hoveredId && !selectedId)
                col = tickHover;
        }

        ImVec2 XTickPosition= tick.imagePosition.to_imvec();

        if (!extend_line(XTickPosition, TargetDirY, LineStart, LineEnd,
                         ImVec2((float) image->get_width(), (float) image->get_height()) - LineMargin * 2, LineMargin))
        {
            LineStart = ImVec2(LineMargin.x, CoordOriginImg.y);
            LineEnd = ImVec2(image->get_width() - LineMargin.x, CoordOriginImg.y);
            //std::cout << "force horizontal\n";
        }
        LineStart = WinPos + im_pos + LineStart*imageScale;
        LineEnd = WinPos + im_pos + LineEnd*imageScale;
        LabelPos = WinPos + im_pos+ (tick.imagePosition * imageScale + Vec2D(10.f, 0.f)).to_imvec();

        //draw_list->AddLine(ImVec2(TickPos, 0.0f)+WinPos, ImVec2(TickPos, canvas_sz.y)+WinPos, col, 1.0f);
        draw_list->AddLine(LineStart, LineEnd, col, 2.0f);

        ImVec2 padding(5.f, 5.f);

        ImVec2 sz = ImGui::CalcTextSize(tick.tickValueStr.c_str());
        draw_list->AddRectFilled(LabelPos-padding, LabelPos+sz+padding, ImColor(255,255,255));

        //ImGui::SetCursorPos(ImVec2(TickPos+2.0f, CoordOriginScreen.y - WinPos.y+2.0f));
        ImGui::SetCursorScreenPos(LabelPos);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 255));
        ImGui::TextUnformatted(tick.tickValueStr.c_str());
        ImGui::PopStyleColor();
    }
    for (auto &tick : YTicks) {
        ImU32 col = tickColor;

        if (currentMode & MODE_GRID)
        {
            if (tick.id == selectedId)
                col = tickSel;
            else if (tick.id == hoveredId && !selectedId)
                col = tickHover;
        }

        ImVec2 YTickPosition= tick.imagePosition.to_imvec();

        if (!extend_line(YTickPosition, TargetDirX, LineStart, LineEnd,
                         ImVec2((float) image->get_width(), (float) image->get_height()) - LineMargin * 2, LineMargin))
        {
            LineStart = ImVec2(LineMargin.x, CoordOriginImg.y);
            LineEnd = ImVec2(image->get_width() - LineMargin.x, CoordOriginImg.y);
            //std::cout << "force horizontal\n";
        }
        LineStart = WinPos + im_pos + LineStart*imageScale;
        LineEnd = WinPos + im_pos + LineEnd*imageScale;
        LabelPos = WinPos + im_pos+ (tick.imagePosition * imageScale + Vec2D(0.f, 10.f)).to_imvec();

        //draw_list->AddLine(ImVec2(TickPos, 0.0f)+WinPos, ImVec2(TickPos, canvas_sz.y)+WinPos, col, 1.0f);
        draw_list->AddLine(LineStart, LineEnd, col, 2.0f);

        ImVec2 padding(5.f, 5.f);

        ImVec2 sz = ImGui::CalcTextSize(tick.tickValueStr.c_str());
        draw_list->AddRectFilled(LabelPos-padding, LabelPos+sz+padding, ImColor(255,255,255));

        ImGui::SetCursorScreenPos(LabelPos);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 255));
        ImGui::TextUnformatted(tick.tickValueStr.c_str());
        ImGui::PopStyleColor();

        //draw_list->AddLine(ImVec2(0.0f, TickPos) + WinPos, ImVec2(canvas_sz.x, TickPos) + WinPos, col, 1.0f);
// 		ImGui::SetCursorPos(ImVec2(CoordOriginScreen.x - WinPos.x + 2.0f, TickPos + 2.0f));
// 		ImGui::PushStyleColor(ImGuiCol_Text, ImColor(0, 0, 0, 255));
// 		ImGui::Text("%0.2f", YTicks[kp].z);
// 		ImGui::PopStyleColor();
    }
}

void MainWindow::render_horizon(const ImVec2 &im_pos)
{
    if(!curve || currentMode != MODE_HORIZON)
        return;

    ImVec2 WinPos = ImGui::GetWindowPos();

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    auto horizon= curve->get_horizon();
    auto selectedId= curve->get_selected_id();
    auto hoveredId= curve->get_hovered_id(ImageElement::HORIZON);

    float pointSize = 12.f;
    auto lineColor = ImColor(128, 128, 128, 255);
    auto pointStroke = ImColor(64, 64, 64, 255);
    auto pointHover = ImColor(255, 255, 255, 255);
    auto pointFill = ImColor(255, 200, 60, 255);
    auto deleteFill = ImColor(205, 92, 92, 255);

    auto CoordOriginImg= horizon.imagePosition.to_imvec();
    auto CoordOriginTargetX= horizon.target.imagePosition.to_imvec();

    ImVec2 CoordOriginScreen = CoordOriginImg*imageScale + im_pos + WinPos;
    ImVec2 CoordTargetScreen = CoordOriginTargetX*imageScale + im_pos + WinPos;

    //draw horizon
    draw_list->AddLine(CoordOriginScreen, CoordTargetScreen, lineColor, 2.0f);

    ImU32 fill = pointFill;

    if (ImageHorizon::ORIGIN == selectedId || (selectedId == 0 && ImageHorizon::ORIGIN == hoveredId))
        fill = deleteOnRelease ? deleteFill : pointHover;

    draw_list->AddCircleFilled(CoordOriginScreen, pointSize*0.5f, pointStroke);
    draw_list->AddCircleFilled(CoordOriginScreen, pointSize*0.5f-1.f, fill);



    fill = pointFill;

    if (ImageHorizon::TARGET == selectedId || (selectedId == 0 && ImageHorizon::TARGET == hoveredId))
        fill = pointHover;

    draw_list->AddCircleFilled(CoordTargetScreen, pointSize*0.5f, pointStroke);
    draw_list->AddCircleFilled(CoordTargetScreen, pointSize*0.5f-1.f, fill);


}

void MainWindow::render_side_panel()
{
    float SettingsWidth = 250.0f;


    ImGui::BeginChild("SettingsWindow", ImVec2(SettingsWidth, 0));

    ImVec2 CurPos = ImGui::GetCursorPos();

    ImGui::SetCursorPosY(CurPos.y + 3.0f);
    ImGui::Text("Subdivision:");
    ImGui::SameLine();
    ImGui::SetCursorPosY(CurPos.y);

    CurPos = ImGui::GetCursorPos();

    ImGui::PushItemWidth(SettingsWidth - CurPos.x);
    ImGui::SliderInt("##subdiv_slider", &subdivLevel, 0, maxSubdivLevel);
    ImGui::PopItemWidth();



    ImGui::Checkbox("Draw Subdivision Points", &bShowSubdivPoints);
    ImGui::Checkbox("Show Image", &bShowImage);
    ImGui::Checkbox("Show Binarization", &bShowBinarization);
    ImGui::Checkbox("Invert Image", &bInvertImage);
    ImGui::Checkbox("Show Zoom Window", &bShowZoomWindow);
    //ImGui::Checkbox("Smooth Subdivision", &bSmoothPoints);


    if (curve)
    {
        CurPos = ImGui::GetCursorPos();

        ImGui::SetCursorPosY(CurPos.y + 3.0f);
        //ImGui::Text("Bin. level");
        ImGui::Text("Threshold: ");
        ImGui::SameLine();
        ImGui::SetCursorPosY(CurPos.y);

        CurPos = ImGui::GetCursorPos();

        ImGui::PushItemWidth(SettingsWidth - CurPos.x);
        ImGui::SliderInt("##bin-level", &binLevel, 0, 255);

        ImGui::PopItemWidth();


        ImGui_ImplOpenGL3_SetBinarizationLevel(binLevel);
        ImGui_ImplOpenGL3_SetInvertImage(bInvertImage);

        curve->set_subdiv_level(subdivLevel);
        curve->set_bin_level(binLevel);
        curve->set_invert_image(bInvertImage);



        //tune curve thickness
        CurPos = ImGui::GetCursorPos();

        ImGui::SetCursorPosY(CurPos.y + 3.0f);
        ImGui::Text("Curve thickness: ");
        ImGui::SameLine();
        ImGui::SetCursorPosY(CurPos.y);
        CurPos = ImGui::GetCursorPos();
        ImGui::PushItemWidth(SettingsWidth - CurPos.x);
        ImGui::SliderInt("##curve-thick", &curveThickness, curveThicknessMin, curveThicknessMax);
        ImGui::PopItemWidth();
        curve->set_curve_thickness(curveThickness);






        CurPos = ImGui::GetCursorPos();
        ImGui::SetCursorPosY(CurPos.y + 3.0f);
        //ImGui::Text("Bin. level");
        ImGui::Text("Scale: ");
        ImGui::SameLine();


        ImGui::SetCursorPosY(CurPos.y);

        CurPos = ImGui::GetCursorPos();

        float NewScale = imageScale;

        ImGui::PushItemWidth(SettingsWidth - CurPos.x);
        ImGui::SliderFloat("##image-scale", &NewScale, minImageScale, maxImageScale, "%.2f");
        ImGui::PopItemWidth();


        //ImVec2 PrevHoveredPixel = (MousePos - WinPos - imagePosition) / imageScale;
        if (NewScale != imageScale)
        {
            //TODO keep center in place
            imageScale = NewScale;
            //imagePosition = MousePos - HoveredPixel*imageScale - WinPos;
        }

        float scale_width=120.f;
        ImGui::PushItemWidth(scale_width);
        const char* scales[] = { "linear", "logarithmic" };
        const CurveDetect::AxisScale scales_enum[] ={
                CurveDetect::LINEAR, CurveDetect::LOG };
        static int xscale = 0;
        static int yscale = 0;
        ImGui::TextUnformatted("X Scale:");
        ImGui::SameLine(SettingsWidth-scale_width);
        ImGui::Combo("##xscale", &xscale, scales, 2);

        ImGui::TextUnformatted("Y Scale:");
        ImGui::SameLine(SettingsWidth-scale_width);
        ImGui::Combo("##yscale", &yscale, scales, 2);

        curve->set_scales(scales_enum[xscale], scales_enum[yscale]);

    }

    float buttonMargin = 3.f;

    if (ImGui::Button("Open", ImVec2(SettingsWidth/2-buttonMargin, 0)))
        on_open_image();

    ImGui::SameLine(0.f,buttonMargin*2);

    if (ImGui::Button("Reset", ImVec2(SettingsWidth/2-buttonMargin, 0)))
        reset_all();

    if (ImGui::Button("Paste Image", ImVec2(SettingsWidth, 0)))
        on_paste_image();

    ImGui::Separator();

    //render text export settings

    int out_Result;
    bool bExportReady = curve ? curve->is_export_ready(out_Result) : false;

    static char edit_buf_col_sep[10];
    static char edit_buf_line_end[10];

    std::string edit_str;

    edit_str = escape(columnSeparator);
    edit_str.copy(edit_buf_col_sep, 8);

    float inputWidth=90.f;

    ImGui::PushItemWidth(inputWidth);
    ImGui::TextUnformatted("Column separator");
    ImGui::SameLine(SettingsWidth-inputWidth);
    ImGui::InputText("##col-sep", edit_buf_col_sep, 6);

    edit_str = edit_buf_col_sep;
    columnSeparator = unescape(edit_str);

    if (columnSeparator.size() == 0)
        columnSeparator = " ";


    edit_str = escape(lineEnding);
    edit_str.copy(edit_buf_line_end, 8);

    ImGui::TextUnformatted("Line ending");
    ImGui::SameLine(SettingsWidth-inputWidth);
    ImGui::InputText("##line-end", edit_buf_line_end, 6);

    edit_str = edit_buf_line_end;
    lineEnding = unescape(edit_str);

    if (lineEnding.size() == 0)
        lineEnding = " ";


    const char* items[] = { "dot", "comma" };
    static int item2 = 0;
    ImGui::TextUnformatted("Decimal separator");
    ImGui::SameLine(SettingsWidth-inputWidth);
    ImGui::Combo("##dec-sep", &item2, items, 2);   // Combo using proper array. You can also pass a callback to retrieve array value, no need to create/copy an array just for that.

    decimalSeparator = item2==0 ? '.' : ',';

    ImGui::PopItemWidth();

    if(!bExportReady)
        ImGui_PushDisableButton();

    if (ImGui::Button("Copy", ImVec2(SettingsWidth/2-buttonMargin, 0)) && bExportReady)
    {
        auto text=curve->get_points_text(columnSeparator, lineEnding, decimalSeparator);
        MainApp::get().copy_to_clipboard(text);
    }

    ImGui::SameLine(0.f,buttonMargin*2);

    if (ImGui::Button("Export", ImVec2(SettingsWidth/2-buttonMargin, 0)) && bExportReady)
        on_export_points();

    if(!bExportReady)
        ImGui_PopDisableButton();


    ImGui::Separator();

    render_hints_panel();

    ImGui::EndChild();
}

void MainWindow::render_hints_panel()
{
    if(!curve)
    {
        ImGui::TextUnformatted("Open image to begin");
        return;
    }

    int out_Result;
    curve->is_export_ready(out_Result);

//    std::string buf;

    if(out_Result & CurveDetect::NO_POINTS || out_Result & CurveDetect::ONE_POINT)
        ImGui::TextUnformatted("Add at least 2 points");


    if(out_Result & CurveDetect::PIXEL_OVERLAP_X_GRID)
        ImGui::TextUnformatted("Vertical grid lines are overlapping");
    else if(out_Result & CurveDetect::VALUE_OVERLAP_X_GRID)
        ImGui::TextUnformatted("Vertical grid lines have the same value");

    if(out_Result & CurveDetect::PIXEL_OVERLAP_Y_GRID)
        ImGui::TextUnformatted("Horizontal grid lines are overlapping");
    else if(out_Result & CurveDetect::VALUE_OVERLAP_Y_GRID)
        ImGui::TextUnformatted("Horizontal grid lines have the same value");

    if(out_Result != CurveDetect::READY)
        ImGui::TextUnformatted("");

    //describe current work mode
    std::string modeStr;
    std::string modifierStr = "(move)";

    auto& app= MainApp::get();

    bool snap = curve ? curve->get_selected_id()!=0 : false;

    if (app.is_ctrl_down())
        modifierStr = snap ? "(snap)" : "(new)";
    else if (deleteOnRelease)
        modifierStr = "(delete)";

    std::string helpStr;

    switch (currentMode)
    {
        case MODE_POINTS:
            modeStr = "Point";
            helpStr = "Drag (or use arrow keys)\nto move a point\nHold Ctrl to add a new point\nShift+Click to delete a point\nHold Ctrl to snap selected point";
            break;
        case MODE_HORIZON:
            modeStr = "Horizon";
            modifierStr = "";
            helpStr = "Drag (or use arrow keys) to change\nhorizon of image\nHold Ctrl to snap selected end";
            break;
        case MODE_GRID:
            modeStr = "Grid";
            modifierStr = "";
            helpStr = "Drag (or use arrow keys)\nto move grid lines\nDouble click to change value\nHold Ctrl to snap selected line";
            break;

    }

    helpStr += "\n\nDrag with middle button to pan\nUse mouse wheel to zoom";

    ImGui::Text("Work mode: %s %s", modeStr.c_str(), modifierStr.c_str());
    ImGui::TextUnformatted("");
    ImGui::TextUnformatted(helpStr.c_str());
}

void MainWindow::on_export_points()
{

    int out_Result;
    if (!curve || !curve->is_export_ready(out_Result))
        return;

    char const * lFilterPatterns[2] = { "*.txt", "*.mat" };

    std::string path="data.txt";

    const char* lTheSaveFileName = tinyfd_saveFileDialog(
            "Choose a save file",
            path.c_str(),
            2,
            lFilterPatterns,
            NULL);


    if (!lTheSaveFileName)
    {
        return;
    }

    path = lTheSaveFileName;

    std::cout << "save: " << path << "\n";

    size_t dot_ind = path.find_last_of('.');

    bool bUseTextFormat = true;

    if (dot_ind != std::string::npos)
    {
        std::string ext = path.substr(dot_ind, path.size() - dot_ind);

        std::cout << "extension: " << ext << "\n";

        if (ext.compare(".txt") == 0)
        {
            std::cout << "text format\n";
        }
        else if (ext.compare(".mat") == 0)
        {
            bUseTextFormat = false;
            std::cout << "mat format\n";
        }
        else
        {
            path.erase(dot_ind, path.size() - dot_ind);
            path.append(".txt");

            if (!tinyfd_messageBox(
                    "Error",
                    "You selected unknown format\nSave file as *.txt?",
                    "yesno",
                    "question",
                    1))
            {
                on_export_points();
                return;
            }

        }
    }
    else
    {
        path.append(".txt");

        if (!tinyfd_messageBox(
                "Error",
                "You selected unknown format\nSave file as *.txt?",
                "yesno",
                "question",
                1))
        {
            on_export_points();
            return;
        }

    }

    std::cout << "save: " << path << "\n";

    if(bUseTextFormat)
    {
        auto text=curve->get_points_text(columnSeparator, lineEnding, decimalSeparator);

        std::ofstream ofs;
        ofs.open(path);
        ofs<<text;
        ofs.close();
    }
    else
        curve->export_points_mat_file(lTheSaveFileName);
}


void MainWindow::on_open_image()
{
    char const * filename;
    char const * lFilterPatterns[3] = { "*.png", "*.jpg", "*.bmp" };

    std::string path;

    filename = tinyfd_openFileDialog(
            "Choose an Image",
            path.c_str(),
            3,
            lFilterPatterns,
            NULL,
            0);

    if (!filename)
        return;


    path = filename;

    std::cout << "open: "<<filename<<"\n";

    image=std::make_shared<Image>(path);

    if(!image->is_loaded())
    {
        tinyfd_messageBox("Can't open image", "Opened file is not an image.\nTry again.", "ok", "error", 0);
        image= nullptr;
        curve= nullptr;
    }
    else
    {
        curve=std::make_shared<CurveDetect>(image);
        currentMode = ActionMode::MODE_POINTS;
    }

    //TODO don't reset if image was not opened
    reset_all();
}


void MainWindow::on_paste_image()
{

    std::cout << "paste from buf\n";

    ImageData imageData;

    if(Clipboard::get().get_image(imageData))
    {
        image=std::make_shared<Image>(imageData);


        curve=std::make_shared<CurveDetect>(image);
        currentMode = ActionMode::MODE_POINTS;
        reset_all();
    } else
    {
        tinyfd_messageBox("Can't paste image", "Clipboard doesn't contain any valid image data\nTry again.", "ok", "error", 0);
    }
}


void MainWindow::reset_all()
{
    imageScale = 0.0f;

    if (curve)
    {
        curve->reset_all();
    }

}


bool MainWindow::extend_line(ImVec2 Point, ImVec2 Direction, ImVec2 &out_Start, ImVec2 &out_End, ImVec2 RegionSize,
                             ImVec2 RegionTL/*=ImVec2(0.0f,0.0f)*/)
{
    out_Start = Point;
    out_End = Point + Direction;


    if (std::abs(Direction.x) + std::abs(Direction.y)<0.0001f)
    {
        return false;
    }

    float A = Direction.y;
    float B = -Direction.x;
    float C = -A*Point.x - B*Point.y;
    float C2 = B*Point.x - A*Point.y;

    ImVec2 RegionBR = RegionTL + RegionSize;//bottom-right corner

    //x line: A*x+B*y+C=0		C=-A*x0-B*y0
    //y line: -B*x+A*y+C2=0;	C2=B*x0-A*y0
    if (std::abs(B) > 1.0f && std::abs(A) > 1.0f && true) //line is NOT vertical and is NOT horizontal
    {
        float lefty, righty, topx, botx;

        righty = (-C - A *RegionBR.x) / B;
        lefty = (-C - A *RegionTL.x) / B;
        botx = (-C - B *RegionBR.y) / A;
        topx = (-C - B *RegionTL.y) / A;

        out_Start = ImVec2(RegionTL.x, lefty);

        if (lefty<RegionTL.y)
            out_Start = ImVec2(topx, RegionTL.y);
        if (lefty>RegionBR.y)
            out_Start = ImVec2(botx, RegionBR.y);

        out_End = ImVec2(RegionBR.x, righty);

        if (righty<RegionTL.y)
            out_End = ImVec2(topx, RegionTL.y);
        if (righty>RegionBR.y)
            out_End = ImVec2(botx, RegionBR.y);

        if (B > 0.0f)
        {
            ImVec2 temp = out_Start;
            out_Start = out_End;
            out_End = temp;
        }
    }
    else if (std::abs(B) < 2.0f && std::abs(A) > 1.0f) // line is vertical
    {
        float topx, botx;

        botx = (-C - B *RegionBR.y) / A;
        topx = (-C - B *RegionTL.y) / A;

        out_Start = ImVec2(botx, RegionBR.y);

        out_End = ImVec2(topx, RegionTL.y);

        if (A > 0.0f)
        {
            ImVec2 temp = out_Start;
            out_Start = out_End;
            out_End = temp;
        }
    }
    else if (std::abs(A) < 2.0f && std::abs(B) > 1.0f) // line is horizontal
    {
        float lefty, righty;

        righty = (-C - A *RegionBR.x) / B;
        lefty = (-C - A *RegionTL.x) / B;

        out_Start = ImVec2(RegionTL.x, lefty);

        out_End = ImVec2(RegionBR.x, righty);

        if (B > 0.0f)
        {
            ImVec2 temp = out_Start;
            out_Start = out_End;
            out_End = temp;
        }
    }
    else
    {
        return false;
    }

    return true;
}


std::string MainWindow::unescape(const std::string& s)
{
    std::string res;
    std::string::const_iterator it = s.begin();
    char c, c2;
    while (it != s.end())
    {
        c = *it++;
        if (c == '\\' && it != s.end())
        {
            c2 = *it++;
            switch (c2)
            {
                case '\\': c = '\\'; break;
                case 'n': c = '\n'; break;
                case 't': c = '\t'; break;
                    // all other escapes
                default:
                    // invalid escape sequence - skip it. alternatively you can copy it as is, throw an exception...
                    //res += c;
                    //c= c2;
                    continue;
                    //break;
            }
        }
        res += c;
    }

    return res;
}

std::string MainWindow::escape(const std::string& s)
{
    std::string res;
    std::string::const_iterator it = s.begin();
    while (it != s.end())
    {
        char c = *it++;

        switch (c)
        {
            case '\\':
                res += '\\';
                res += '\\';
                break;
            case '\n':
                res += '\\';
                res += 'n';
                break;
            case '\t':
                res += '\\';
                res += 't';
                break;
                // all other escapes
            default:
                res += c;
                // invalid escape sequence - skip it. alternatively you can copy it as is, throw an exception...
                continue;
        }

    }

    return res;
}


void MainWindow::ImGui_PopDisableButton()
{
    ImGui::PopStyleColor(3);
}

void MainWindow::ImGui_PushDisableButton()
{
    ImGui::PushStyleColor(ImGuiCol_Button, colorDisabled.Value);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, colorDisabled.Value);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorDisabled.Value);
}

