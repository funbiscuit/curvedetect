
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
    
    CurrentMode = ActionMode0_None;
    
    
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
    static ImVec2 HoveredPixel = ImVec2(0.0f, 0.0f);
    
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    
    ShowSidePanel();
    
    ImGui::SameLine();
    
    
    ImGui::BeginGroup();
    
    
    // Create our canvas
    if(image)
        ImGui::Text("Hovered pixel (%d,%d) value: %d", (int)HoveredPixel.x, (int)HoveredPixel.y,
                    image->getPixelValue((int)HoveredPixel.x, (int)HoveredPixel.y));
    
    //ImGui::SameLine(ImGui::GetWindowWidth() - 100);
    
    //ImGui::Checkbox("Show grid", &show_grid);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImVec4(30, 30, 30, 255));
    ImGui::BeginChild("image_region", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
    
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    //draw_list->ChannelsSplit(2);
    
    
    ImVec2 canvas_sz = ImGui::GetWindowSize();
    ImVec2 WinPos = ImGui::GetWindowPos();
    ImVec2 MousePos = ImGui::GetMousePos();
    
    if(image)
    {
        int im_width = image->get_width();
        int im_height = image->get_height();
        
        float im_aspect = float(im_height) / float(im_width);
        
        GLuint texID = image->get_texture();
        
        float scaleX, scaleY;
        
        scaleX = canvas_sz.x / float(image->get_width());
        scaleY = canvas_sz.y / float(image->get_height());
        
        MinImageScale = scaleX < scaleY ? scaleX : scaleY;
        
        
        ImVec2 PrevHoveredPixel = (MousePos - WinPos - CurrentImPos) / CurrentImageScale;
        
        bool bScaleChanged = false;
        
        if (ImGui::IsMouseHoveringWindow())
        {
            ImGuiIO& imIO = ImGui::GetIO();
            
            if (imIO.MouseWheel < 0)
            {
                float mul = std::pow(1.1f, imIO.MouseWheel);
                CurrentImageScale *= mul;
                bScaleChanged = true;
                //std::cout << mul << "\n";
            }
            else if (imIO.MouseWheel > 0)
            {
                float mul = std::pow(1.1f, imIO.MouseWheel);
                CurrentImageScale *= std::pow(1.1f, imIO.MouseWheel);
                bScaleChanged = true;
                //std::cout << mul << "\n";
            }
        }
        
        //std::cout <<imIO.MouseWheel << "\n";
        
        //MinImageScale = float(im_width) / float(image->GetWidth());
        if (CurrentImageScale < MinImageScale)
            CurrentImageScale = MinImageScale;
        if (CurrentImageScale > MaxImageScale)
            CurrentImageScale = MaxImageScale;
        
        
        //ImVec2 PrevHoveredPixel = (MousePos - WinPos - CurrentImPos) / CurrentImageScale;
        if(bScaleChanged)
            CurrentImPos = MousePos - HoveredPixel*CurrentImageScale - WinPos;
        
        ImVec2 MinImPos, MaxImPos;
        MinImPos.x = canvas_sz.x - im_width*CurrentImageScale;
        MinImPos.y = canvas_sz.y - im_height*CurrentImageScale;
        
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
        
        //if(CurrentImPos.x<MinImPos.x)
        
        //CurrentImPos = MinImPos;
        
        im_width = int(float(im_width)*CurrentImageScale);
        im_height = int(float(im_height)*CurrentImageScale);
        
        //CurrentImageScale = float(im_width )/ float(image->GetWidth());
        
        
        ImVec2 cur_pos = ImGui::GetCursorScreenPos();
        if (bShowImage)
        {
            
            //ImDrawCallback cb = &testFunc;
            //draw_list->AddCallback(cb, 0);
            
            draw_list->AddImage((void *)(intptr_t)(texID), cur_pos + CurrentImPos, cur_pos + CurrentImPos + ImVec2((float)im_width, (float)im_height));
        }
        
        //this is used to distinguish in shader
        //between our image (that should be binarized)
        //and all other textures (that should be rendered without changes)
        ImGui_ImplOpenGL3_SetImageTexID(bShowBinarization ? texID : -1);
        
    }
    
    
    
    HoveredPixel = MousePos - WinPos - CurrentImPos;
    
    HoveredPixel.x /= CurrentImageScale;
    HoveredPixel.y /= CurrentImageScale;
    
    if(curve)
        curve->UpdateHoveredItemIndex(Vec2D(HoveredPixel), CurrentMode);
    
    ProcessInput(HoveredPixel);
    
    
    ShowCoordSystem(CurrentImPos);
    
    
    
    ShowTickLines(CurrentImPos);
    
    
    ShowPoints(CurrentImageScale, CurrentImPos, MousePos);
    
    ImVec2 ZoomOrigin;
    
    
    
    if (image && ShowZoomWindow(canvas_sz, HoveredPixel, ZoomOrigin))
    {
        float zoomedScale = float(ZoomWndSize)/ float(2 * ZoomPixelHSide + 1);
        
        ImVec2 zoomedImPos = ZoomOrigin -HoveredPixel*zoomedScale-WinPos;
        
        ImVec2 ZoomWnd = ImVec2(ZoomWndSize, ZoomWndSize);
        
        draw_list->PushClipRect(ZoomOrigin- ZoomWnd*0.5f, ZoomOrigin+ ZoomWnd*0.5f, true);
        ShowPoints(zoomedScale, zoomedImPos, ZoomOrigin);
        draw_list->PopClipRect();
        
    }
    
    
    
    
    // Draw context menu
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
    if (ImGui::BeginPopup("context_menu"))
    {
        ImVec2 scene_pos = ImGui::GetMousePosOnOpeningCurrentPopup();
        bIsContextMenuOpened = true;
        bIsReadyForAction = false;
        
        ImGui::Text("Choose work mode");
        ImGui::Separator();
        
        const char* items[]={"[0] None","[1] Point (Add/Move/Delete)","[2] Origin (Move)",
                             "[3] X Target (Move)","[4] X Tick (Add/Move/Delete)","[5] Y Tick (Add/Move/Delete)"};
        ActionMode modes[]={ActionMode0_None,ActionMode1_AddPoints,ActionMode2_MoveOrigin,
                            ActionMode3_MoveXTarget,ActionMode4_AddXTick,ActionMode5_AddYTick};
        
        for(int j=0;j<6;++j)
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


void MainWindow::ProcessInput(ImVec2 &HoveredPixel)
{
    static bool bIsMouseDownFirst = true;
    
    
    //std::cout << ImGui::IsWindowFocused() << "\n";
    
    if (ImGui::IsWindowFocused())
    {
        
        ImGuiIO& io = ImGui::GetIO();
        
        if (io.KeysDown[GLFW_KEY_0] || io.KeysDown[GLFW_KEY_KP_0])
        {
            CurrentMode = ActionMode0_None;
        }
        else if (io.KeysDown[GLFW_KEY_1] || io.KeysDown[GLFW_KEY_KP_1])
        {
            CurrentMode = ActionMode1_AddPoints;
        }
        else if (io.KeysDown[GLFW_KEY_2] || io.KeysDown[GLFW_KEY_KP_2])
        {
            CurrentMode = ActionMode2_MoveOrigin;
        }
        else if (io.KeysDown[GLFW_KEY_3] || io.KeysDown[GLFW_KEY_KP_3])
        {
            CurrentMode = ActionMode3_MoveXTarget;
        }
        else if (io.KeysDown[GLFW_KEY_4] || io.KeysDown[GLFW_KEY_KP_4])
        {
            CurrentMode = ActionMode4_AddXTick;
        }
        else if (io.KeysDown[GLFW_KEY_5] || io.KeysDown[GLFW_KEY_KP_5])
        {
            CurrentMode = ActionMode5_AddYTick;
        }
    }
    
    auto& app=MainApp::getInstance();
    
    if (curve && ImGui::IsMouseHoveringWindow() && !bIsContextMenuOpened && bIsReadyForAction)
    {
        if (ImGui::IsMouseDown(0))
        {
            if (bIsMouseDownFirst) //first press
            {
                if (image->isPixelInside((int)HoveredPixel.x, (int)HoveredPixel.y))
                {
                    switch (CurrentMode)
                    {
                        case ActionMode0_None:
                            break;
                        case ActionMode1_AddPoints:
                            if (app.isCtrlPressed())
                            {
                                curve->AddPoint(Vec2D(HoveredPixel));
                            }
                            else if (curve->GetHoveredPoint()!=-1)
                            {
                                if (app.isShiftPressed())
                                {
                                    curve->DeleteHoveredPoint();
                                }
                                else
                                {
                                    curve->SelectHovered();
                                }
                            }
                            break;
                        case ActionMode2_MoveOrigin:
                            curve->SetOrigin(Vec2D(HoveredPixel), app.isCtrlPressed());
                            break;
                        case ActionMode3_MoveXTarget:
                            curve->SetTarget(Vec2D(HoveredPixel), app.isCtrlPressed());
                            break;
                        case ActionMode4_AddXTick:
                            if (app.isCtrlPressed() && curve->AddXTick(Vec2D(HoveredPixel)))
                            {
                            }
                            else if (curve->GetHoveredXTick()!=-1)
                            {
                                if (app.isShiftPressed())
                                {
                                    curve->DeleteHoveredXTick();
                                }
                                else
                                {
                                    curve->SelectHovered();
                                }
                            }
                            break;
                        case ActionMode5_AddYTick:
                            if (app.isCtrlPressed() && curve->AddYTick(Vec2D(HoveredPixel)))
                            {
                            }
                            else if (curve->GetHoveredYTick()!=-1)
                            {
                                if (app.isShiftPressed())
                                {
                                    curve->DeleteHoveredYTick();
                                }
                                else
                                {
                                    curve->SelectHovered();
                                }
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
            else//continued press
            {
                if (image->isPixelInside((int)HoveredPixel.x, (int)HoveredPixel.y))
                {
                    ImVec2 SnappedPoint;
                    
                    switch (CurrentMode)
                    {
                        case ActionMode0_None:
                            break;
                        case ActionMode1_AddPoints:
                            if (curve->GetSelected()>=0)
                            {
                                curve->MoveSelectedPoint(Vec2D(HoveredPixel), app.isCtrlPressed());
                            }
                            break;
                        case ActionMode2_MoveOrigin:
                            curve->SetOrigin(Vec2D(HoveredPixel), app.isCtrlPressed());
                            break;
                        case ActionMode3_MoveXTarget:
                            curve->SetTarget(Vec2D(HoveredPixel), app.isCtrlPressed());
                            break;
                        case ActionMode4_AddXTick:
                            if (curve->GetSelected()>=0)
                            {
                                curve->MoveSelectedXTick(Vec2D(HoveredPixel), app.isCtrlPressed());
                            }
                            break;
                        case ActionMode5_AddYTick:
                            if (curve->GetSelected()>=0)
                            {
                                curve->MoveSelectedYTick(Vec2D(HoveredPixel), app.isCtrlPressed());
                            }
                            break;
                        default:
                            break;
                    }
                    
                }
            }
            
            
            bIsMouseDownFirst = false;
        }
        if (ImGui::IsMouseReleased(0))
        {
            switch (CurrentMode)
            {
                case ActionMode0_None:
                    break;
                case ActionMode1_AddPoints:
                    if (curve->GetSelected()>=0)
                    {
                        
                        if (image->isPixelInside((int)HoveredPixel.x, (int)HoveredPixel.y))
                        {
                            curve->MoveSelectedPoint(Vec2D(HoveredPixel), app.isCtrlPressed());
                        }
                        
                        
                        curve->Deselect();
                    }
                    break;
                case ActionMode2_MoveOrigin:
                    break;
                case ActionMode4_AddXTick:
                    if (curve->GetSelected()>=0)
                    {
                        ImGui::OpenPopup("TickConfig");
                    }
                    break;
                case ActionMode5_AddYTick:
                    if (curve->GetSelected()>=0)
                    {
                        ImGui::OpenPopup("TickConfig");
                    }
                    break;
                default:
                    break;
            }
            
            
            
            
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
    
    
}

void MainWindow::ShowPoints(float im_scale, ImVec2 im_pos, ImVec2 MousePos)
{
    if(!curve)
        return;
    
    ImVec2 WinPos = ImGui::GetWindowPos();
    
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    int SelectedItem=curve->GetSelected();
    int HoveredItem=curve->GetHovered();
    auto& allPoints=curve->GetAllPoints();
    auto& userPoints=curve->GetUserPoints();
    
    //draw lines between subdivided points
    if (allPoints.size() > 1)
    {
        for (size_t kp = 0; kp < allPoints.size() - 1; kp++)
        {
            ImVec2 PointPos0 = allPoints[kp].imagePosition.ToImVec2() * im_scale + im_pos + WinPos;
            ImVec2 PointPos1 = allPoints[kp + 1].imagePosition.ToImVec2() * im_scale + im_pos + WinPos;
            
            ImU32 LineColor = ImColor(80, 255, 80, 200);
            draw_list->AddLine(PointPos0, PointPos1, LineColor, 2.0f);
            
            if (bDrawSubdivideMarkers)
            {
                draw_list->AddCircleFilled(PointPos0, 4.0f, ImColor(80, 255, 80, 200), 8);
                draw_list->AddCircle(PointPos0, 5.0f, ImColor(0, 0, 0, 200), 8, 2.0f);
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
    for (size_t kp = 0; kp < userPoints.size(); kp++)
    {
        ImVec2 PointPos = userPoints[kp].imagePosition.ToImVec2() * im_scale + im_pos + WinPos;
        
        ImU32 CircleFill = ImColor(150, 150, 150, 255);
        
        if (CurrentMode == ActionMode1_AddPoints)
        {
            if (kp == SelectedItem || (SelectedItem == -1 && kp == HoveredItem))
            {
                
                CircleFill = ImColor(255, 255, 255, 255);
                if (!app.isCtrlPressed() && app.isShiftPressed() && !ImGui::IsMouseDown(0))
                {
                    CircleFill = ImColor(255, 20, 20, 255);
                    
                }
            }
        }
        
        draw_list->AddCircleFilled(PointPos, 4.0f, CircleFill);
        draw_list->AddCircle(PointPos, 5.0f, ImColor(0, 0, 0, 200), 12, 2.0f);
    }
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
        
        static int TickType = 0; //X
        static int TickPrecision = 3; //X
        static double TickValue = 0;
    
        int SelectedItem=curve->GetSelected();
        auto& XTicks=curve->GetXTicks();
        auto& YTicks=curve->GetYTicks();
    
        if (bTickConfigInit)
        {
            if (CurrentMode == ActionMode4_AddXTick)//X
            {
                TickValue = XTicks[SelectedItem].tickValue;
            }
            else if (CurrentMode == ActionMode5_AddYTick)//Y
            {
                TickValue = YTicks[SelectedItem].tickValue;
            }
            
            bTickConfigInit = false;
        }
        
        //ImGui::RadioButton("X (vertical)", &TickType, 0);
        //ImGui::RadioButton("Y (horizontal)", &TickType, 1);
        
        ImGui::InputInt("Precision", &TickPrecision);
        
        TickPrecision = TickPrecision < 0 ? 0 : (TickPrecision > 8 ? 8 : TickPrecision);
        
        //if (bTickInputAutoFocus)//set focus to float only when we just opened the window
        //	ImGui::SetKeyboardFocusHere(0);
        
        if (!ImGui::IsAnyItemActive())
            ImGui::SetKeyboardFocusHere();

        //TODO use filtered text input
        ImGui::InputDouble("Value", &TickValue, 0.0, 0.0);
        
        auto& app=MainApp::getInstance();
        
        if (ImGui::Button("OK", ImVec2(120, 0)) || app.isEnterReleased())
        {
            //std::cout << TickValue << ", " << SelectedItem << '\n';
            
            if (CurrentMode == ActionMode4_AddXTick)//X
            {
                XTicks[SelectedItem].tickValue = TickValue;
            }
            else if (CurrentMode == ActionMode5_AddYTick)//Y
            {
                YTicks[SelectedItem].tickValue = TickValue;
            }
            
            SelectedItem = -1;
            bTickConfigInit = true;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            //TODO restore from backup
            if (CurrentMode == ActionMode4_AddXTick)//X
            {
                //XTicks.pop_back();
            }
            else if (CurrentMode == ActionMode5_AddYTick)//Y
            {
                //YTicks.pop_back();
            }
            
            SelectedItem = -1;
            
            bTickConfigInit = true;
            ImGui::CloseCurrentPopup();
        }
        
        
        if (ImGui::Button("Delete tick line", ImVec2(248, 0)))
        {
            
            if (CurrentMode == ActionMode4_AddXTick)//X
            {
                XTicks.erase(XTicks.begin() + SelectedItem);
            }
            else if (CurrentMode == ActionMode5_AddYTick)//Y
            {
                YTicks.erase(YTicks.begin() + SelectedItem);
                //YTicks.pop_back();
            }
            
            SelectedItem = -1;
            
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

bool MainWindow::ShowZoomWindow(const ImVec2 &canvas_sz, const ImVec2 &HoveredPixel, ImVec2& out_ZoomOrigin)
{
    ImVec2 WinPos = ImGui::GetWindowPos();
    ImVec2 MousePos = ImGui::GetMousePos();
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    float im_width = float(image->get_width())*CurrentImageScale;
    float im_height = float(image->get_height())*CurrentImageScale;
    
    float im_aspect = im_height / im_width;
    
    
    ImVec2 ZoomUV0, ZoomUV1, ZoomPos, ZoomWndOffset;
    float ZoomRectBorder = 5.0f;
    
    ZoomWndOffset = ImVec2(15.f, 15.f);
    ZoomPos = MousePos;
    
    if (ZoomPos.x + ZoomWndOffset.x + ZoomWndSize + ZoomRectBorder * 2 - WinPos.x > canvas_sz.x)
        ZoomPos.x = canvas_sz.x - (ZoomWndOffset.x + ZoomWndSize + ZoomRectBorder * 2 - WinPos.x);
    
    if (ZoomPos.y + ZoomWndOffset.y + ZoomWndSize + ZoomRectBorder * 2 - WinPos.y > canvas_sz.y)
        ZoomPos.y = canvas_sz.y - (ZoomWndOffset.y + ZoomWndSize + ZoomRectBorder * 2 - WinPos.y);
    
    
    if (ZoomPos.x + ZoomWndOffset.x - ZoomRectBorder * 2 - WinPos.x < 0)
        ZoomPos.x = -(ZoomWndOffset.x - ZoomRectBorder * 2 - WinPos.x);
    
    if (ZoomPos.y + ZoomWndOffset.y - ZoomRectBorder * 2 - WinPos.y < 0)
        ZoomPos.y = -(ZoomWndOffset.y - ZoomRectBorder * 2 - WinPos.y);
    
    
    
    
    ZoomUV0.x = HoveredPixel.x / image->get_width();
    ZoomUV0.y = HoveredPixel.y / image->get_height();
    
    
    out_ZoomOrigin = ZoomPos + ZoomWndOffset + ImVec2(ZoomWndSize*0.5f,ZoomWndSize*0.5f);
    
    float ZoomUVsideU = float(2 * ZoomPixelHSide + 1) / float(image->get_width());
    
    
    ZoomUV0 -= ImVec2(ZoomUVsideU, ZoomUVsideU / im_aspect)*0.5f;
    ZoomUV1 = ZoomUV0 + ImVec2(ZoomUVsideU, ZoomUVsideU / im_aspect);
    
    //draw zoom window
    if (ImGui::IsMouseHoveringWindow() && CurrentMode != ActionMode0_None)
    {
        ImU32 ZoomBgColor = ImColor(120, 120, 120, 220);
        draw_list->AddRectFilled(ZoomPos + ZoomWndOffset - ImVec2(ZoomRectBorder, ZoomRectBorder),
                                 ZoomPos + ZoomWndOffset + ImVec2(ZoomRectBorder+ ZoomWndSize, ZoomRectBorder+ ZoomWndSize),
                                 ZoomBgColor, 5.0f);
        
        GLuint texID = image->get_texture();
        draw_list->AddImage((void *)(intptr_t)(texID), ZoomPos + ZoomWndOffset,
                            ZoomPos + ZoomWndOffset + ImVec2(ZoomWndSize,ZoomWndSize), ZoomUV0, ZoomUV1);
        
        ImU32 CrossColor = ImColor(120, 120, 120, 220);
        draw_list->AddLine(ImVec2(ZoomWndOffset.x + ZoomWndSize*0.5f, ZoomWndOffset.y) + ZoomPos,
                           ImVec2(ZoomWndOffset.x + ZoomWndSize*0.5f, ZoomWndSize + ZoomWndOffset.y) + ZoomPos, CrossColor);
        draw_list->AddLine(ImVec2(ZoomWndOffset.x, ZoomWndSize*0.5f + ZoomWndOffset.y) + ZoomPos,
                           ImVec2(ZoomWndOffset.x + ZoomWndSize, ZoomWndSize*0.5f + ZoomWndOffset.y) + ZoomPos, CrossColor);
        
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
    
    ImU32 TickColor = ImColor(120, 120, 120, 255);
    
    auto horizon=curve->GetHorizon();
    auto CoordOriginImg=horizon.imagePosition.ToImVec2();
    auto CoordOriginTargetX=horizon.targetPosition.ToImVec2();
    int HoveredItem=curve->GetHovered();
    auto& XTicks=curve->GetXTicks();
    auto& YTicks=curve->GetYTicks();
    
    ImVec2 TargetDirX = CoordOriginTargetX - CoordOriginImg;
    
    ImVec2 TargetDirY;
    TargetDirY.x = TargetDirX.y;
    TargetDirY.y = -TargetDirX.x;
    
    ImVec2 LineStart, LineEnd, LabelPos;
    
    ImVec2 LineMargin = ImVec2(10.0f, 10.0f);
    
    //draw tick lines
    for (size_t kp = 0; kp < XTicks.size(); kp++)
    {
        //float TickPos = (XTicks[kp]).x * im_scale + im_pos.x;
        
        ImU32 col = TickColor;
        
        if (kp == HoveredItem && CurrentMode == ActionMode4_AddXTick)
        {
            col = ImColor(255, 20, 20, 255);
        }
        
        ImVec2 XTickPosition=XTicks[kp].imagePosition.ToImVec2();
        
        if (!MakeFullLine(XTickPosition, TargetDirY, LineStart, LineEnd,
                          ImVec2((float)image->get_width(), (float)image->get_height()) - LineMargin * 2, LineMargin))
        {
            LineStart = ImVec2(LineMargin.x, CoordOriginImg.y);
            LineEnd = ImVec2(image->get_width() - LineMargin.x, CoordOriginImg.y);
            //std::cout << "force horizontal\n";
        }
        
        
        float beta = (LineEnd.x - LineStart.x)*(CoordOriginImg.y - LineStart.y)
                     - (LineEnd.y - LineStart.y)*(CoordOriginImg.x - LineStart.x);
        
        beta /= TargetDirX.x*(LineEnd.y - LineStart.y) - TargetDirX.y*(LineEnd.x - LineStart.x);
        
        LabelPos = CoordOriginImg + TargetDirX*beta;
        
        LineStart = WinPos + im_pos + LineStart*CurrentImageScale;
        LineEnd = WinPos + im_pos + LineEnd*CurrentImageScale;
        LabelPos = im_pos + LabelPos*CurrentImageScale;
        
        //draw_list->AddLine(ImVec2(TickPos, 0.0f)+WinPos, ImVec2(TickPos, canvas_sz.y)+WinPos, col, 1.0f);
        draw_list->AddLine(LineStart, LineEnd, col, 2.0f);
        //ImGui::SetCursorPos(ImVec2(TickPos+2.0f, CoordOriginScreen.y - WinPos.y+2.0f));
        ImGui::SetCursorPos(LabelPos);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 255));
        //TODO better format (maybe store users input string)
        ImGui::Text("%0.2f", XTicks[kp].tickValue);
        ImGui::PopStyleColor();
    }
    for (size_t kp = 0; kp < YTicks.size(); kp++)
    {
        //float TickPos = (YTicks[kp]).y * im_scale + im_pos.y;
        
        ImU32 col = TickColor;
        
        if (kp == HoveredItem && CurrentMode == ActionMode5_AddYTick)
        {
            col = ImColor(255, 20, 20, 255);
        }

        ImVec2 YTickPosition=YTicks[kp].imagePosition.ToImVec2();
        
        if (!MakeFullLine(YTickPosition, TargetDirX, LineStart, LineEnd,
                          ImVec2((float)image->get_width(), (float)image->get_height()) - LineMargin * 2, LineMargin))
        {
            LineStart = ImVec2(LineMargin.x, CoordOriginImg.y);
            LineEnd = ImVec2(image->get_width() - LineMargin.x, CoordOriginImg.y);
            //std::cout << "force horizontal\n";
        }
        
        
        float beta = (LineEnd.x - LineStart.x)*(CoordOriginImg.y - LineStart.y)
                     - (LineEnd.y - LineStart.y)*(CoordOriginImg.x - LineStart.x);
        
        beta /= TargetDirY.x*(LineEnd.y - LineStart.y) - TargetDirY.y*(LineEnd.x - LineStart.x);
        
        LabelPos = CoordOriginImg + TargetDirY*beta;
        
        LineStart = WinPos + im_pos + LineStart*CurrentImageScale;
        LineEnd = WinPos + im_pos + LineEnd*CurrentImageScale;
        LabelPos = im_pos + LabelPos*CurrentImageScale;
        
        //draw_list->AddLine(ImVec2(TickPos, 0.0f)+WinPos, ImVec2(TickPos, canvas_sz.y)+WinPos, col, 1.0f);
        draw_list->AddLine(LineStart, LineEnd, col, 2.0f);
        //ImGui::SetCursorPos(ImVec2(TickPos+2.0f, CoordOriginScreen.y - WinPos.y+2.0f));
        ImGui::SetCursorPos(LabelPos);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 255));
        //TODO better format (maybe store users input string)
        ImGui::Text("%0.2f", YTicks[kp].tickValue);
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
    if(!curve)
        return;
    
    ImVec2 WinPos = ImGui::GetWindowPos();
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    float im_width = float(image->get_width())*CurrentImageScale;

    auto horizon=curve->GetHorizon();
    auto CoordOriginImg=horizon.imagePosition.ToImVec2();
    auto CoordOriginTargetX=horizon.targetPosition.ToImVec2();
    auto& XTicks=curve->GetXTicks();
    auto& YTicks=curve->GetYTicks();
    
    ImVec2 CoordOriginScreen = CoordOriginImg*CurrentImageScale + im_pos + WinPos;
    
    //draw axis
    ImU32 AxisColor = ImColor(120, 120, 120, 220);
    
    //x axis
    ImVec2 LineStart = ImVec2(WinPos.x + im_pos.x + 10.0f, CoordOriginScreen.y);
    ImVec2 LineEnd = ImVec2(WinPos.x + im_pos.x + im_width - 10.0f, CoordOriginScreen.y);
    
    ImVec2 TargetDirX = CoordOriginTargetX - CoordOriginImg;
    
    ImVec2 LineMargin = ImVec2(10.0f, 10.0f);
    
    if (!MakeFullLine(CoordOriginImg, TargetDirX, LineStart, LineEnd,
                      ImVec2((float)image->get_width(), (float)image->get_height()) - LineMargin * 2, LineMargin))
    {
        LineStart = ImVec2(LineMargin.x, CoordOriginImg.y);
        LineEnd = ImVec2(image->get_width() - LineMargin.x, CoordOriginImg.y);
        //std::cout << "force horizontal\n";
    }
    
    LineStart = WinPos + im_pos + LineStart*CurrentImageScale;
    LineEnd = WinPos + im_pos + LineEnd*CurrentImageScale;
    
    draw_list->AddLine(LineStart, LineEnd, AxisColor, 3.0f);
    draw_list->AddCircleFilled(LineEnd, 6.0f, AxisColor);
    
    //draw arrow
    //LineStart = LineEnd - ImVec2(20.0f, 8.0f);
    //draw_list->AddLine(LineStart, LineEnd, AxisColor, 2.0f);
    //LineStart = LineEnd - ImVec2(20.0f, -8.0f);
    //draw_list->AddLine(LineStart, LineEnd, AxisColor, 2.0f);
    
    ImVec2 LabelPos = LineEnd + ImVec2(-10.0f, 25.0f);
    
    ImVec2 LabelSize = ImVec2(10.0f, 20.0f);
    
    //draw X
    draw_list->AddLine(LabelPos - LabelSize*0.5f, LabelPos + LabelSize*0.5f, AxisColor, 2.0f);
    draw_list->AddLine(LabelPos - LabelSize * ImVec2(1.0f, -1.0f) * 0.5f, LabelPos + LabelSize * ImVec2(1.0f, -1.0f) * 0.5f, AxisColor, 2.0f);
    
    
    
    
    //y axis
    
    ImVec2 TargetDirY;
    TargetDirY.x = TargetDirX.y;
    TargetDirY.y = -TargetDirX.x;
    
    if (!MakeFullLine(CoordOriginImg, TargetDirY, LineStart, LineEnd,
                      ImVec2((float)image->get_width(), (float)image->get_height()) - LineMargin * 2, LineMargin))
    {
        LineEnd = ImVec2(CoordOriginImg.x, LineMargin.y);
        LineStart = ImVec2(CoordOriginImg.x, image->get_height() - LineMargin.y);
    }
    
    LineStart = WinPos + im_pos + LineStart*CurrentImageScale;
    LineEnd = WinPos + im_pos + LineEnd*CurrentImageScale;
    draw_list->AddLine(LineStart, LineEnd, AxisColor, 3.0f);
    draw_list->AddCircleFilled(LineEnd, 6.0f, AxisColor);
    
    //draw arrow
    //draw_list->AddLine(LineStart, LineEnd, AxisColor, 2.0f);
    //LineStart = LineEnd + ImVec2(8.0f, 20.0f);
    //draw_list->AddLine(LineStart, LineEnd, AxisColor, 2.0f);
    //LineStart = LineEnd + ImVec2(-8.0f, 20.0f);
    //draw_list->AddLine(LineStart, LineEnd, AxisColor, 2.0f);
    
    LabelPos = LineEnd + ImVec2(-25.0f, 10.0f);
    
    //draw Y
    draw_list->AddLine(LabelPos - LabelSize*0.5f, LabelPos + ImVec2(0.0f, 2.0f), AxisColor, 2.0f);
    draw_list->AddLine(LabelPos - LabelSize * ImVec2(-1.0f, 1.0f) * 0.5f, LabelPos + ImVec2(0.0f, 2.0f), AxisColor, 2.0f);
    draw_list->AddLine(LabelPos + ImVec2(0.0f, 2.0f), LabelPos + LabelSize * ImVec2(0.0f, 0.5f), AxisColor, 2.0f);
    
    
    
    //draw arrow
    //LineStart = LineEnd - ImVec2(20.0f, 8.0f);
    //draw_list->AddLine(LineStart, LineEnd, AxisColor, 2.0f);
    //LineStart = LineEnd - ImVec2(20.0f, -8.0f);
    //draw_list->AddLine(LineStart, LineEnd, AxisColor, 2.0f);
}

void MainWindow::ShowSidePanel()
{
    float SettingsWidth = 250.0f;
    
    
    ImGui::BeginChild("SettingsWindow", ImVec2(SettingsWidth, 0));
    ImGui::Text("Settings");
    
    int SubdivideIterations=0;
    
    if(curve)
        SubdivideIterations=curve->SubdivideIterations;
    
    int PrevSubdivideIterations = SubdivideIterations;
// 	ImGui::PushItemWidth(65.0f);
// 	ImGui::InputInt("Subdivision", &SubdivideIterations);
// 	ImGui::PopItemWidth();
    
    ImVec2 CurPos = ImGui::GetCursorPos();
    
    ImGui::SetCursorPosY(CurPos.y + 3.0f);
    ImGui::Text("Subdivision");
    ImGui::SameLine();
    ImGui::SetCursorPosY(CurPos.y);
    ImGui::SetCursorPosX(SettingsWidth -30.0f*2-5.0f);
    
    if (PrevSubdivideIterations <=0 || !curve)
        ImGui_PushDisableButton();
    if (ImGui::Button("-", ImVec2(30.f, 0)) && SubdivideIterations>0)
    {
        SubdivideIterations--;
    }
    if (PrevSubdivideIterations <=0 || !curve)
        ImGui_PopDisableButton();
    
    ImGui::SameLine();
    ImGui::SetCursorPosX(SettingsWidth - 30.0f);
    
    if (PrevSubdivideIterations >=CurveDetect::MaxSubdivideIterations || !curve)
        ImGui_PushDisableButton();
    if (ImGui::Button("+", ImVec2(30.f, 0)) && SubdivideIterations<CurveDetect::MaxSubdivideIterations)
    {
        SubdivideIterations++;
    }
    if (PrevSubdivideIterations >=CurveDetect::MaxSubdivideIterations || !curve)
        ImGui_PopDisableButton();

// 	if (SubdivideIterations > MaxSubdivideIterations)
// 		SubdivideIterations = MaxSubdivideIterations;
// 	if (SubdivideIterations < 0)
// 		SubdivideIterations = 0;
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 WinPos = ImGui::GetWindowPos();
    CurPos = ImGui::GetCursorPos();
    
    float barTotalWidth = SettingsWidth;
    float barSpacing = 5.0f;
    float barWidth = (barTotalWidth - (CurveDetect::MaxSubdivideIterations - 1)*barSpacing) / float(CurveDetect::MaxSubdivideIterations);
    
    for (int i = 0; i < CurveDetect::MaxSubdivideIterations; i++)
    {
        ImVec2 PointPos0 = ImVec2((barWidth + barSpacing)*i, 5.0f) + WinPos + CurPos;
        ImVec2 PointPos1 = ImVec2((barWidth + barSpacing)*i+barWidth, 5.0f) + WinPos + CurPos;
        ImVec2 PointPos2 = ImVec2(barWidth+barSpacing, 5.0f) + WinPos + CurPos;
        ImU32 LineColor = ImColor(200, 200, 200, 255);
        
        if(i<SubdivideIterations)
            LineColor= ImColor(154, 204, 255, 255);//TODO primary color
        
        draw_list->AddLine(PointPos0, PointPos1, LineColor, 6.0f);
        
        
    }
    
    ImGui::SetCursorPosY(CurPos.y + 15.0f);
    
    
    
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
        case ActionMode0_None:
            modeStr = "None";
            modifierStr = "";
            helpStr = "Press right mouse button\nto choose work mode";
            break;
        case ActionMode1_AddPoints:
            modeStr = "Point";
            helpStr = "Drag (or use arrow keys)\nto move a point\nHold Ctrl to add a new point\nHold Shift to delete a point\nHold Ctrl to enable snapping";
            break;
        case ActionMode2_MoveOrigin:
            modeStr = "Origin";
            modifierStr = "(move)";
            helpStr = "Drag (or use arrow keys) to move\norigin of the coordinate system\nHold Ctrl to enable snapping";
            break;
        case ActionMode3_MoveXTarget:
            modeStr = "X Target";
            modifierStr = "(move)";
            helpStr = "Drag (or use arrow keys)\nto choose direction for the\nX axis of the coordinate system\nHold Ctrl to enable snapping";
            break;
        case ActionMode4_AddXTick:
            modeStr = "X Tick";
            helpStr = "Drag (or use arrow keys)\nto move X tick line\nHold Ctrl to add new X tick\nHold Shift to delete X tick\nHold Ctrl to enable snapping";
            break;
        case ActionMode5_AddYTick:
            modeStr = "Y Tick";
            helpStr = "Drag (or use arrow keys)\nto move Y tick line\nHold Ctrl to add new Y tick\nHold Shift to delete Y tick\nHold Ctrl to enable snapping";
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
        curve=std::make_shared<CurveDetect>(image);
    
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

