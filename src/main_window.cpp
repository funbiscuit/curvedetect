
#include <iostream>
#include <string>
#include <vector>

#include "main_window.h"
#include "main_app.h"
#include "imgui.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_helpers.h"
#include "glad/glad.h"

#include <GLFW/glfw3.h>

#include "tinyfiledialogs.h"

MainWindow::MainWindow()
{
    width=1024;
    height=720;
    toolbar_width=280;
    
    CurrentMode = MODE_NONE;
    
    
    MinImageScale = 1.0f;
    CurrentImageScale = 1.0f;
    MaxImageScale = 3.0f;
    
    ZoomPixelHSide = 30;
    
    ZoomWndSize = 200.0f;
    
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
        ShowMainWindow();
        ImGui::End();
    }
    ImGui::PopStyleVar();
    
}

void MainWindow::init()
{
#ifndef NDEBUG
    image=std::make_shared<Image>("../img/test.png");
    curve=std::make_shared<CurveDetect>(image);
    curve->ResetAll();
#endif
}

void MainWindow::on_resize(int w, int h)
{
    width=w; height=h;
}

void MainWindow::ShowMainWindow()
{
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    
    ShowSidePanel();
    
    ImGui::SameLine();
    
    
    ImGui::BeginGroup();
    
    
    // Create our canvas
    if(image)
        ImGui::Text("Hovered pixel (%d,%d) value: %d", (int)HoveredPixel.x, (int)HoveredPixel.y,
                    image->getPixelValue((int)HoveredPixel.x, (int)HoveredPixel.y));

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImVec4(30, 30, 30, 255));
    ImGui::BeginChild("image_region", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);

    ImVec2 canvas_sz = ImGui::GetWindowSize();

    ShowImage(canvas_sz);

    ImVec2 WinPos = ImGui::GetWindowPos();
    ImVec2 MousePos = ImGui::GetMousePos();
    HoveredPixel = (MousePos - WinPos - CurrentImPos)/CurrentImageScale;

    if(curve)
        curve->UpdateHoveredItem(Vec2D(HoveredPixel));
    
    ProcessInput();
    
    
    ShowCoordSystem(CurrentImPos);
    ShowTickLines(CurrentImPos);
    ShowPoints(CurrentImageScale, CurrentImPos, MousePos);
    
    ImVec2 ZoomOrigin;
    
    if (image && ShowZoomWindow(canvas_sz, ZoomOrigin))
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        float zoomedScale = ZoomWndSize/ float(2 * ZoomPixelHSide + 1);
        
        ImVec2 zoomedImPos = ZoomOrigin -HoveredPixel*zoomedScale-WinPos;
        
        ImVec2 ZoomWnd = ImVec2(ZoomWndSize, ZoomWndSize);

        //TODO add ticks and horizon
        //draw zoomed curve in zoom window
        draw_list->PushClipRect(ZoomOrigin- ZoomWnd*0.5f, ZoomOrigin+ ZoomWnd*0.5f, true);
        ShowPoints(zoomedScale, zoomedImPos, ZoomOrigin);
        draw_list->PopClipRect();
        
    }
    
    
    
    
    // Draw context menu
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
    if (ImGui::BeginPopup("context_menu"))
    {
        bIsContextMenuOpened = true;
        bIsReadyForAction = false;
        
        const char* items[]={"[1] Points","[2] Horizon",
                             "[3] Ticks"};
        ActionMode modes[]={MODE_POINTS,MODE_HORIZON,MODE_TICKS};
        
        for(int j=0;j<3;++j)
        {
            if (ImGui::MenuItem(items[j], nullptr, CurrentMode == modes[j], true))
            {
                CurrentMode = modes[j];
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
    
    
    ShowTickConfigPopup();
    
    
    
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
    ImGui::EndGroup();
}

void MainWindow::ShowImage(ImVec2 canvasSize)
{
    if(image)
    {
        float im_width = image->get_width();
        float im_height = image->get_height();

        GLuint texID = image->get_texture();

        float scaleX, scaleY;

        scaleX = canvasSize.x / im_width;
        scaleY = canvasSize.y / im_height;

        MinImageScale = scaleX < scaleY ? scaleX : scaleY;

        bool bScaleChanged = false;

        if (ImGui::IsMouseHoveringWindow())
        {
            ImGuiIO& imIO = ImGui::GetIO();

            if (imIO.MouseWheel != 0)
            {
                CurrentImageScale *= std::pow(1.1f, imIO.MouseWheel);
                bScaleChanged = true;
            }
        }

        if (CurrentImageScale < MinImageScale)
            CurrentImageScale = MinImageScale;
        if (CurrentImageScale > MaxImageScale)
            CurrentImageScale = MaxImageScale;

        ImVec2 WinPos = ImGui::GetWindowPos();
        ImVec2 MousePos = ImGui::GetMousePos();
        if(bScaleChanged)
            CurrentImPos = MousePos - HoveredPixel*CurrentImageScale - WinPos;

        ImVec2 MinImPos, MaxImPos;
        MinImPos.x = canvasSize.x - im_width*CurrentImageScale;
        MinImPos.y = canvasSize.y - im_height*CurrentImageScale;

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
            CurrentImPos += ImGui::GetMouseDragDelta(2);
            ImGui::ResetMouseDragDelta(2);
        }

        CurrentImPos.x = CurrentImPos.x < MinImPos.x ? MinImPos.x : (CurrentImPos.x > MaxImPos.x ? MaxImPos.x : CurrentImPos.x);
        CurrentImPos.y = CurrentImPos.y < MinImPos.y ? MinImPos.y : (CurrentImPos.y > MaxImPos.y ? MaxImPos.y : CurrentImPos.y);

        im_width*=CurrentImageScale;
        im_height*=CurrentImageScale;

        if (bShowImage)
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 cur_pos = ImGui::GetCursorScreenPos()+CurrentImPos;
            draw_list->AddImage((void *)(intptr_t)(texID), cur_pos, cur_pos + ImVec2(im_width, im_height));
        }

        //this is used to distinguish in shader
        //between our image (that should be binarized)
        //and all other textures (that should be rendered without changes)
        ImGui_ImplOpenGL3_SetImageBin(texID, bShowBinarization);
    }
}

void MainWindow::OnMouseDown(int btn)
{
    auto& app=MainApp::getInstance();
    if (btn == 0)
    {
        switch (CurrentMode)
        {
            case MODE_POINTS:
                if (app.isCtrlPressed())
                {
                    curve->AddPoint(Vec2D(HoveredPixel));
                }
                else if (curve->SelectHovered(ImageElement::POINT))
                {
                    curve->MoveSelected(Vec2D(HoveredPixel));
                    curve->SortPoints();
                    curve->UpdateSubdivision();
                }
                break;
            case MODE_HORIZON:
                if (curve->SelectHovered(ImageElement::HORIZON))
                {
                    curve->MoveSelected(Vec2D(HoveredPixel));
                    curve->SortPoints();
                    curve->UpdateSubdivision();
                }
                break;
            case MODE_TICKS:
                if (curve->SelectHovered(ImageElement::TICKS))
                {
                    //TODO process double click and show input window
                    curve->BackupSelectedTick();
                }
                break;
            default:
                break;
        }
    }
}

void MainWindow::OnMouseUp(int btn)
{
    auto& app=MainApp::getInstance();
    if (btn == 0)
    {
        switch (CurrentMode)
        {
            case MODE_POINTS:
            case MODE_HORIZON:
                if (deleteOnRelease)
                    curve->DeleteSelected();
                else
                    curve->DeselectAll();
                break;
            case MODE_TICKS:
                curve->DeselectAll();
                break;
            default:
                break;
        }
    }
}

void MainWindow::OnMouseDoubleClick(int btn)
{
    auto& app=MainApp::getInstance();
    if (btn == 0)
    {
        switch (CurrentMode)
        {
            case MODE_TICKS:
                if(curve->GetSelectedId())
                    ImGui::OpenPopup("TickConfig");
                break;
            default:
                break;
        }
    }
}

void MainWindow::OnMouseDrag(int btn)
{
    auto& app=MainApp::getInstance();
    if (btn == 0 && curve)
    {
        deleteOnRelease = false;

        if(curve->MoveSelected(Vec2D(HoveredPixel)))
        {
            if(app.isCtrlPressed())
                curve->SnapSelected();

            if(CurrentMode == MODE_POINTS || CurrentMode == MODE_HORIZON)
            {
                curve->SortPoints();
                curve->UpdateSubdivision();
            }
        }
    }
}

void MainWindow::ProcessInput()
{
    static bool bIsMouseDownFirst = true;
    static ImVec2 lastMousePos = ImGui::GetMousePos();
    ImVec2 delta = ImGui::GetMousePos()-lastMousePos;
    bool moving = (std::abs(delta.x)+std::abs(delta.y))>0.f;
    auto& app=MainApp::getInstance();


    if (ImGui::IsWindowFocused())
    {
        ImGuiIO& io = ImGui::GetIO();
        ActionMode modes[]={MODE_POINTS,MODE_HORIZON,MODE_TICKS};

        for(int j=0;j<3;++j)
        {
            if (io.KeysDown[GLFW_KEY_1+j] || io.KeysDown[GLFW_KEY_KP_1+j])
            {
                CurrentMode = modes[j];
            }
        }

        if(curve && curve->GetSelectedId()==0)
            deleteOnRelease = app.isShiftPressed();
    }

    if (curve && ImGui::IsMouseHoveringWindow() && !bIsContextMenuOpened && bIsReadyForAction)
    {
        if (ImGui::IsMouseDown(0) && image->isPixelInside((int)HoveredPixel.x, (int)HoveredPixel.y))
        {
            if (bIsMouseDownFirst) //first press
            {
                OnMouseDown(0);
            }
            else if(moving)
            {
                OnMouseDrag(0);
            }
            bIsMouseDownFirst = false;
        }
        if (ImGui::IsMouseReleased(0))
        {
            OnMouseUp(0);
            
            bIsMouseDownFirst = true;
        }
        if(ImGui::IsMouseDoubleClicked(0))
        {
            OnMouseDoubleClick(0);
            bIsMouseDownFirst = true;
        }
        
        // Open context menu
        if (ImGui::IsMouseClicked(1))
        {
            ImGui::OpenPopup("context_menu");
        }
        
    }
    
    if(curve)
        curve->CheckTarget();
    
    if (ImGui::IsMouseReleased(0) && !bIsContextMenuOpened)
    {
        bIsReadyForAction = true;
    }
    lastMousePos+=delta;
    
}

void MainWindow::ShowPoints(float im_scale, ImVec2 im_pos, ImVec2 MousePos)
{
    if(!curve)
        return;
    
    ImVec2 WinPos = ImGui::GetWindowPos();
    
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    uint64_t selectedId=curve->GetSelectedId();
    uint64_t hoveredId=curve->GetHoveredId(ImageElement::POINT);
    auto& allPoints=curve->GetAllPoints();
    auto& userPoints=curve->GetUserPoints();

    float subdivSize = 8.f;
    float userSize = 12.f;
    auto lineColor = ImColor(128, 128, 128, 255);
    auto pointStroke = ImColor(64, 64, 64, 255);
    auto userHover = ImColor(255, 255, 255, 255);
    auto userFill = ImColor(124, 252, 0, 255);
    auto subdivFill = ImColor(128, 128, 128, 255);
    auto subdivBadFill = ImColor(205, 92, 92, 255);

    //draw lines between subdivided points
    if (allPoints.size() > 1)
    {
        for (size_t kp = 0; kp < allPoints.size() - 1; kp++)
        {
            ImVec2 PointPos0 = allPoints[kp].imagePosition.ToImVec2() * im_scale + im_pos + WinPos;
            ImVec2 PointPos1 = allPoints[kp + 1].imagePosition.ToImVec2() * im_scale + im_pos + WinPos;

            draw_list->AddLine(PointPos0, PointPos1, lineColor, 1.5f);

            ImU32 fill = allPoints[kp].isSnapped ? subdivFill : subdivBadFill;
            
            if (bDrawSubdivideMarkers)
            {
                draw_list->AddCircleFilled(PointPos0, subdivSize*0.5f, pointStroke);
                draw_list->AddCircleFilled(PointPos0, subdivSize*0.5f-1.f, fill);
            }
        }
    }
    
    
    //TODO maybe not needed
    //if we are pressing button - draw a line from press location to snapped point
//    if (SelectedItem >= 0 && ImGui::IsMouseDown(0) && CurrentMode == ActionMode1_AddPoints
//        && SelectedItem < int(UserPoints.size()) && ImGui::IsMouseHoveringWindow())
//    {
//        ImVec2 PointPos0 = UserPoints[SelectedItem] * im_scale + im_pos + WinPos;
//
//        ImU32 LineColor = ImColor(80, 220, 80, 220);
//        draw_list->AddLine(PointPos0, MousePos, LineColor, 2.0f);
//    }

    auto& app=MainApp::getInstance();
    for (const auto &point : userPoints) {
        ImVec2 PointPos = point.imagePosition.ToImVec2() * im_scale + im_pos + WinPos;

        ImU32 fill = userFill;

        if (CurrentMode == MODE_POINTS)
            if (point.id == selectedId || (selectedId == 0 && point.id == hoveredId))
                fill = userHover;

        draw_list->AddCircleFilled(PointPos, userSize*0.5f, pointStroke);
        draw_list->AddCircleFilled(PointPos, userSize*0.5f-1.f, fill);
    }
}

int onTickInput(ImGuiInputTextCallbackData *data)
{
    std::string tickVal = data->Buf;
    ImageTickLine::filterValueStr(tickVal, false);
    strcpy(data->Buf,tickVal.c_str());
    data->BufTextLen=tickVal.length();
    if(data->CursorPos>tickVal.length())
        data->CursorPos=tickVal.length();
    data->BufDirty=true;
    return 0;
}

void MainWindow::ShowTickConfigPopup()
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
    
        auto selected=(ImageTickLine*) curve->GetSelected();
        auto& XTicks=curve->GetXTicks();
        auto& YTicks=curve->GetYTicks();
    
        if (bTickConfigInit && selected)
        {
            tickVal = selected->tickValueStr;
            ImageTickLine::filterValueStr(tickVal, false);
            
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
        
        auto& app=MainApp::getInstance();
        
        if (ImGui::Button("OK", ImVec2(120, 0)) || app.isEnterReleased())
        {
            ImageTickLine::filterValueStr(tickVal, true);

            if(selected)
                selected->setValueStr(tickVal.c_str());

            curve->DeselectAll();

            bTickConfigInit = true;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {

            if(selected)
                selected->RestoreBackup();

            curve->DeselectAll();
            
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

bool MainWindow::ShowZoomWindow(const ImVec2 &canvas_sz, ImVec2& out_ZoomOrigin)
{
    if(!image)
        return false;

    ImVec2 WinPos = ImGui::GetWindowPos();
    ImVec2 MousePos = ImGui::GetMousePos();
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    float im_width = float(image->get_width())*CurrentImageScale;
    float im_height = float(image->get_height())*CurrentImageScale;
    
    float im_aspect = im_height / im_width;
    
    
    ImVec2 ZoomUV0, ZoomUV1, ZoomPos, ZoomWndOffset;
    float ZoomRectBorder = 5.0f;
    
    ZoomWndOffset = ImVec2(15.f, 15.f);
    ZoomPos = MousePos+ZoomWndOffset;
    
    if (ZoomPos.x + ZoomWndSize + ZoomRectBorder * 2 - WinPos.x > canvas_sz.x)
        ZoomPos.x = MousePos.x - (ZoomWndSize + ZoomRectBorder * 2);

    if (ZoomPos.y + ZoomWndSize + ZoomRectBorder * 2 - WinPos.y > canvas_sz.y)
        ZoomPos.y = MousePos.y - (ZoomWndSize + ZoomRectBorder * 2);
    
    if (ZoomPos.x - ZoomRectBorder * 2 - WinPos.x < 0)
        ZoomPos.x = ZoomRectBorder * 2 + WinPos.x;
    
    if (ZoomPos.y - ZoomRectBorder * 2 - WinPos.y < 0)
        ZoomPos.y = ZoomRectBorder * 2 + WinPos.y;

    ZoomUV0 = HoveredPixel;
    ZoomUV0.x /= image->get_width();
    ZoomUV0.y /= image->get_height();
    
    out_ZoomOrigin = ZoomPos + ImVec2(ZoomWndSize*0.5f,ZoomWndSize*0.5f);
    
    float ZoomUVsideU = float(2 * ZoomPixelHSide + 1) / float(image->get_width());
    
    
    ZoomUV0 -= ImVec2(ZoomUVsideU, ZoomUVsideU / im_aspect)*0.5f;
    ZoomUV1 = ZoomUV0 + ImVec2(ZoomUVsideU, ZoomUVsideU / im_aspect);
    
    //draw zoom window
    if (ImGui::IsMouseHoveringWindow() && CurrentMode != MODE_NONE)
    {
        ImU32 ZoomBgColor = ImColor(120, 120, 120, 220);
        draw_list->AddRectFilled(ZoomPos - ImVec2(ZoomRectBorder, ZoomRectBorder),
                                 ZoomPos + ImVec2(ZoomRectBorder+ ZoomWndSize, ZoomRectBorder+ ZoomWndSize),
                                 ZoomBgColor, 5.0f);
        
        GLuint texID = image->get_texture();
        draw_list->AddImage((void *)(intptr_t)(texID), ZoomPos,
                            ZoomPos + ImVec2(ZoomWndSize,ZoomWndSize), ZoomUV0, ZoomUV1);
        
        ImU32 CrossColor = ImColor(120, 120, 120, 220);
        draw_list->AddLine(ImVec2(ZoomWndSize*0.5f, 0.f) + ZoomPos,
                           ImVec2(ZoomWndSize*0.5f, ZoomWndSize) + ZoomPos, CrossColor);
        draw_list->AddLine(ImVec2(0.f, ZoomWndSize*0.5f) + ZoomPos,
                           ImVec2(ZoomWndSize, ZoomWndSize*0.5f) + ZoomPos, CrossColor);
        
        return true;
    }
    
    return false;
}

void MainWindow::ShowTickLines(ImVec2 im_pos)
{
    if(!curve)
        return;
    
    ImVec2 WinPos = ImGui::GetWindowPos();
    
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    const auto tickColor = ImColor(128, 128, 128, 255);
    const auto tickHover = ImColor(152, 248, 59, 255);
    const auto tickSel = ImColor(59, 155, 59, 255);

    
    auto horizon=curve->GetHorizon();
    auto CoordOriginImg=horizon.imagePosition.ToImVec2();
    auto CoordOriginTargetX=horizon.target.imagePosition.ToImVec2();
    uint64_t hoveredId=curve->GetHoveredId(ImageElement::TICKS);
    uint64_t selectedId=curve->GetSelectedId();
    auto& XTicks=curve->GetXTicks();
    auto& YTicks=curve->GetYTicks();
    
    ImVec2 TargetDirX = CoordOriginTargetX - CoordOriginImg;
    
    ImVec2 TargetDirY;
    TargetDirY.x = TargetDirX.y;
    TargetDirY.y = -TargetDirX.x;
    
    ImVec2 LineStart, LineEnd, LabelPos;
    
    ImVec2 LineMargin = ImVec2(10.0f, 10.0f);
    
    //draw tick lines
    for (auto &tick : XTicks) {
        ImU32 col = tickColor;

        if (CurrentMode & MODE_TICKS)
        {
            if (tick.id == selectedId)
                col = tickSel;
            else if (tick.id == hoveredId && !selectedId)
                col = tickHover;
        }
        
        ImVec2 XTickPosition= tick.imagePosition.ToImVec2();
        
        if (!MakeFullLine(XTickPosition, TargetDirY, LineStart, LineEnd,
                          ImVec2((float)image->get_width(), (float)image->get_height()) - LineMargin * 2, LineMargin))
        {
            LineStart = ImVec2(LineMargin.x, CoordOriginImg.y);
            LineEnd = ImVec2(image->get_width() - LineMargin.x, CoordOriginImg.y);
            //std::cout << "force horizontal\n";
        }
        LineStart = WinPos + im_pos + LineStart*CurrentImageScale;
        LineEnd = WinPos + im_pos + LineEnd*CurrentImageScale;
        LabelPos = WinPos + im_pos+(tick.imagePosition*CurrentImageScale + Vec2D(10.f,0.f)).ToImVec2();

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

        if (CurrentMode & MODE_TICKS)
        {
            if (tick.id == selectedId)
                col = tickSel;
            else if (tick.id == hoveredId && !selectedId)
                col = tickHover;
        }

        ImVec2 YTickPosition= tick.imagePosition.ToImVec2();
        
        if (!MakeFullLine(YTickPosition, TargetDirX, LineStart, LineEnd,
                          ImVec2((float)image->get_width(), (float)image->get_height()) - LineMargin * 2, LineMargin))
        {
            LineStart = ImVec2(LineMargin.x, CoordOriginImg.y);
            LineEnd = ImVec2(image->get_width() - LineMargin.x, CoordOriginImg.y);
            //std::cout << "force horizontal\n";
        }
        LineStart = WinPos + im_pos + LineStart*CurrentImageScale;
        LineEnd = WinPos + im_pos + LineEnd*CurrentImageScale;
        LabelPos = WinPos + im_pos+(tick.imagePosition*CurrentImageScale + Vec2D(0.f,10.f)).ToImVec2();

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

void MainWindow::ShowCoordSystem(const ImVec2 &im_pos)
{
    if(!curve || CurrentMode != MODE_HORIZON)
        return;
    
    ImVec2 WinPos = ImGui::GetWindowPos();
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    auto horizon=curve->GetHorizon();
    auto selectedId=curve->GetSelectedId();
    auto hoveredId=curve->GetHoveredId(ImageElement::HORIZON);

    float pointSize = 12.f;
    auto lineColor = ImColor(128, 128, 128, 255);
    auto pointStroke = ImColor(64, 64, 64, 255);
    auto pointHover = ImColor(255, 255, 255, 255);
    auto pointFill = ImColor(255, 200, 60, 255);

    auto CoordOriginImg=horizon.imagePosition.ToImVec2();
    auto CoordOriginTargetX=horizon.target.imagePosition.ToImVec2();
    
    ImVec2 CoordOriginScreen = CoordOriginImg*CurrentImageScale + im_pos + WinPos;
    ImVec2 CoordTargetScreen = CoordOriginTargetX*CurrentImageScale + im_pos + WinPos;

    //draw horizon
    draw_list->AddLine(CoordOriginScreen, CoordTargetScreen, lineColor, 2.0f);

    ImU32 fill = pointFill;

    if (ImageHorizon::ORIGIN == selectedId || (selectedId == 0 && ImageHorizon::ORIGIN == hoveredId))
        fill = pointHover;

    draw_list->AddCircleFilled(CoordOriginScreen, pointSize*0.5f, pointStroke);
    draw_list->AddCircleFilled(CoordOriginScreen, pointSize*0.5f-1.f, fill);



    fill = pointFill;

    if (ImageHorizon::TARGET == selectedId || (selectedId == 0 && ImageHorizon::TARGET == hoveredId))
        fill = pointHover;

    draw_list->AddCircleFilled(CoordTargetScreen, pointSize*0.5f, pointStroke);
    draw_list->AddCircleFilled(CoordTargetScreen, pointSize*0.5f-1.f, fill);


}

void MainWindow::ShowSidePanel()
{
    float SettingsWidth = 250.0f;
    
    
    ImGui::BeginChild("SettingsWindow", ImVec2(SettingsWidth, 0));
    ImGui::Text("Settings");
    
    int SubdivideIterations=0;
    
    if(curve)
        SubdivideIterations=curve->SubdivideIterations;

    ImVec2 CurPos = ImGui::GetCursorPos();

    ImGui::SetCursorPosY(CurPos.y + 3.0f);
    ImGui::Text("Subdivision:");
    ImGui::SameLine();
    ImGui::SetCursorPosY(CurPos.y);

    CurPos = ImGui::GetCursorPos();

    ImGui::PushItemWidth(SettingsWidth - CurPos.x);
    ImGui::SliderInt("##subdiv_slider", &SubdivideIterations, 0, 6);
    ImGui::PopItemWidth();
    
    
    
    ImGui::Checkbox("Draw Markers", &bDrawSubdivideMarkers);
    ImGui::Checkbox("Show Image", &bShowImage);
    ImGui::Checkbox("Show Binarization", &bShowBinarization);
    //ImGui::Checkbox("Smooth Subdivision", &bSmoothPoints);
    
    
    if (curve)
    {
        int BinarizationLevel = curve->BinarizationLevel;
        int PrevBinarizationLevel = BinarizationLevel;
        
        CurPos = ImGui::GetCursorPos();
        
        ImGui::SetCursorPosY(CurPos.y + 3.0f);
        //ImGui::Text("Bin. level");
        ImGui::Text("Threshold: ");
        ImGui::SameLine();
        ImGui::SetCursorPosY(CurPos.y);
        
        CurPos = ImGui::GetCursorPos();
        
        ImGui::PushItemWidth(SettingsWidth - CurPos.x);
        ImGui::PushID("bin_level_slider");
        ImGui::SliderInt("", &BinarizationLevel, 0, 255);
        
        ImGui::PopItemWidth();
        
        ImGui::PopID();
        
        ImGui_ImplOpenGL3_SetBinarizationLevel(BinarizationLevel);
        
        curve->SetSubdivIterations(SubdivideIterations);
        curve->SetBinarizationLevel(BinarizationLevel);
        
        
        
        CurPos = ImGui::GetCursorPos();
        ImGui::SetCursorPosY(CurPos.y + 3.0f);
        //ImGui::Text("Bin. level");
        ImGui::Text("Scale: ");
        ImGui::SameLine();
        
        
        ImGui::SetCursorPosY(CurPos.y);
        
        CurPos = ImGui::GetCursorPos();
        
        float NewScale = CurrentImageScale;
        
        ImGui::PushItemWidth(SettingsWidth - CurPos.x);
        ImGui::PushID("image_scale_slider");
        ImGui::SliderFloat("", &NewScale, MinImageScale, MaxImageScale, "%.2f");
        ImGui::PopItemWidth();
        
        
        //ImVec2 PrevHoveredPixel = (MousePos - WinPos - CurrentImPos) / CurrentImageScale;
        if (NewScale != CurrentImageScale)
        {
            //TODO keep center in place
            CurrentImageScale = NewScale;
            //CurrentImPos = MousePos - HoveredPixel*CurrentImageScale - WinPos;
        }
        
        
        ImGui::PopID();
    }
    
    
    
    if (ImGui::Button("Open Image", ImVec2(SettingsWidth, 0)))
    {
        OpenImage();
    }
    
    int out_Result;
    bool bExportReady = curve ? curve->IsReadyForExport(out_Result) : false;
    
    if(!bExportReady)
    {
        ImGui_PushDisableButton();
    }
    if (ImGui::Button("Export", ImVec2(SettingsWidth, 0)) && bExportReady)
    {
        ExportPoints();
    }
    if (ImGui::Button("Copy to Clipboard", ImVec2(SettingsWidth, 0)) && bExportReady)
    {
        curve->ExportToClipboard(columnSeparator, lineEnding, decimalSeparator);
    }
    
    if(!bExportReady)
        ImGui_PopDisableButton();
    
    
    if (ImGui::Button("Reset All", ImVec2(SettingsWidth, 0)))
    {
        ResetAll();
    }
    
    ImGui::Separator();
    
    ImGui::Text("Export settings");
    
    static char edit_buf[10];
    static char edit_buf2[10];
    
    static std::string edit_str;
    
    edit_str= escape(columnSeparator);
    
    //compiler is really nervous about this
//#pragma warning( disable : 4996 )
    edit_str.copy(edit_buf, 8);
//#pragma warning( default : 4996 )
    
    ImGui::PushItemWidth(50.0f);
    ImGui::InputText("Column separator", edit_buf, 6);
    
    edit_str = edit_buf;
    columnSeparator = unescape(edit_str);
    
    if (columnSeparator.size() == 0)
        columnSeparator = " ";
    
    
    edit_str = escape(lineEnding);
    edit_str.copy(edit_buf2, 8);
    
    ImGui::InputText("Line ending", edit_buf2, 6);
    
    edit_str = edit_buf2;
    lineEnding = unescape(edit_str);
    
    if (lineEnding.size() == 0)
        lineEnding = " ";
    
    
    const char* items[] = { ".", "," };
    static int item2 = 0;
    ImGui::Combo("Decimal separator", &item2, items, IM_ARRAYSIZE(items));   // Combo using proper array. You can also pass a callback to retrieve array value, no need to create/copy an array just for that.
    
    decimalSeparator = items[item2][0];
    //std::cout << decimalSeparator << "\n";
    
    ImGui::PopItemWidth();
    
    
    
    
    ImGui::Separator();
    
    std::string buf = "";
    
    
    buf = out_Result == ExportReadyStatus_Ready ? "Yes" : "No";
    ImGui::Text("Ready for export: %s", buf.c_str());
    
    
    buf = out_Result & ExportReadyStatus_NoPoints ? "Not enough" : "Ready";
    
    ImGui::Text("Points: %s", buf.c_str());
    
    if (out_Result & ExportReadyStatus_NoXTicks)
    {
        buf = "Not enough";
    }
    else if (out_Result & ExportReadyStatus_XTicksSimilarPositions)
    {
        buf = "Overlapping positions";
    }
    else
    {
        buf = out_Result & ExportReadyStatus_XTicksSimilarValues ? "Overlapping values" : "Ready";
    }
    
    ImGui::Text("X Ticks: %s", buf.c_str());
    
    
    if (out_Result & ExportReadyStatus_NoYTicks)
    {
        buf = "Not enough";
    }
    else if (out_Result & ExportReadyStatus_YTicksSimilarPositions)
    {
        buf = "Overlapping positions";
    }
    else
    {
        buf = out_Result & ExportReadyStatus_YTicksSimilarValues ? "Overlapping values" : "Ready";
    }
    
    ImGui::Text("Y Ticks %s", buf.c_str());
    
    
    ImGui::Separator();
    
    std::string modeStr;
    
    std::string modifierStr = "(move)";
    
    auto& app=MainApp::getInstance();
    
    if (app.isCtrlPressed())
    {
        modifierStr = "(new)";
    }
    else if (app.isShiftPressed())
    {
        modifierStr = "(delete)";
    }
    
    std::string helpStr;
    
    switch (CurrentMode)
    {
        case MODE_NONE:
            modeStr = "None";
            modifierStr = "";
            helpStr = "Press right mouse button\nto choose work mode";
            break;
        case MODE_POINTS:
            modeStr = "Point";
            helpStr = "Drag (or use arrow keys)\nto move a point\nHold Ctrl to add a new point\nHold Shift to delete a point\nHold Ctrl to enable snapping";
            break;
        case MODE_HORIZON:
            modeStr = "Horizon";
            modifierStr = "";
            helpStr = "Drag (or use arrow keys) to change\nhorizon of image\nHold Ctrl to enable snapping";
            break;
        case MODE_TICKS:
            modeStr = "Ticks";
            modifierStr = "";
            helpStr = "Drag (or use arrow keys)\nto move tick lines\nDouble click to change value\nHold Ctrl to enable snapping";
            break;
        
    }
    
    helpStr += "\n\nDrag with middle button to pan\nUse mouse wheel to zoom";
    
    ImGui::Text("Work mode: %s %s", modeStr.c_str(), modifierStr.c_str());
    ImGui::Text("");
    ImGui::Text(helpStr.c_str());
    
    ImGui::EndChild();
}

void MainWindow::ExportPoints()
{
    
    int out_Result;
    if (!curve || !curve->IsReadyForExport(out_Result))
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
                ExportPoints();
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
            ExportPoints();
            return;
        }
        
    }
    
    std::cout << "save: " << path << "\n";
    
    curve->ExportPoints(lTheSaveFileName, bUseTextFormat);
}


void MainWindow::OpenImage()
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
    }
    else
    {
        curve=std::make_shared<CurveDetect>(image);
        CurrentMode = ActionMode::MODE_POINTS;
    }
    
    ResetAll();
}


void MainWindow::ResetAll()
{
    CurrentImageScale = 0.0f;
    
    if (curve)
    {
        curve->ResetAll();
    }
    
}


bool MainWindow::MakeFullLine(ImVec2 Point, ImVec2 Direction, ImVec2& out_Start, ImVec2& out_End, ImVec2 RegionSize, ImVec2 RegionTL/*=ImVec2(0.0f,0.0f)*/)
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
    ImGui::PushStyleColor(ImGuiCol_Button, ColorDisabled);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ColorDisabled);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ColorDisabled);
}

