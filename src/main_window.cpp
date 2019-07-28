
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>

#include <cstdlib>
#include <algorithm>

#include "main_window.h"
#include "main_app.h"
#include "imgui.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_helpers.h"
#include "glad/glad.h"

#include <GLFW/glfw3.h>
#include <filesystem>

#include "tinyfiledialogs.h"
#include "mat_file_writer.h"

#include <Eigen/Dense>
using Eigen::MatrixXd;
using Eigen::MatrixXi;

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
    
    BinarizationLevel = 127;
    
    SubdivideIterations = 3;
    
    SelectedItem = -1;
    HoveredItem = -1;
    
    decimalSeparator = '.';
    columnSeparator = "\t";
    lineEnding = "\n";
    
    image = nullptr;
    
    CoordOriginImg = ImVec2(200, 200);
    
    CoordOriginTargetX = ImVec2(400, 200);
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
    
    
    UpdateHoveredItemIndex(CurrentImPos);
    
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
    
    if (image && ImGui::IsMouseHoveringWindow() && !bIsContextMenuOpened && bIsReadyForAction)
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
                                UserPoints.push_back(HoveredPixel);
                                SnapToCurve(UserPoints.back());
                                SnapToBary(UserPoints.back());
                                SortPoints();
                                UpdateSubdivision();
                                SelectedItem = UserPoints.size() - 1;
                                std::cout << "ctrl was pressed\n";
                            }
                            else if (HoveredItem >= 0 && HoveredItem < int(UserPoints.size()))
                            {
                                if (app.isShiftPressed())
                                {
                                    UserPoints.erase(UserPoints.begin() + HoveredItem);
                                    HoveredItem = -1;
                                    SelectedItem = -1;
                                    SortPoints();
                                    UpdateSubdivision();
                                    //std::cout << "delete";
                                }
                                else
                                {
                                    SelectedItem = HoveredItem;
                                }
                                std::cout << "shift was pressed\n";
                            }
                            break;
                        case ActionMode2_MoveOrigin:
                            CoordOriginTargetX += HoveredPixel - CoordOriginImg;
                            CoordOriginImg = HoveredPixel;
                            break;
                        case ActionMode3_MoveXTarget:
                            CoordOriginTargetX = HoveredPixel;
                            SortPoints();
                            UpdateSubdivision();
                            break;
                        case ActionMode4_AddXTick:
                            if (app.isCtrlPressed() && XTicks.size() < 2)
                            {
                                float value = 0.0f;

#ifdef _DEBUG
                                
                                value = (float) XTicks.size();
							if (value > 0.5f)
							{
								value = XTicks[0].z + 1.0f;
							}
#endif // _DEBUG
                                
                                
                                XTicks.push_back(ImVec4(HoveredPixel.x, HoveredPixel.y, value, 0.0f));
                                SelectedItem = XTicks.size() - 1;
                            }
                            else if (HoveredItem >= 0 && HoveredItem < int(XTicks.size()))
                            {
                                if (app.isShiftPressed())
                                {
                                    XTicks.erase(XTicks.begin() + HoveredItem);
                                    HoveredItem = -1;
                                    SelectedItem = -1;
                                }
                                else
                                {
                                    SelectedItem = HoveredItem;
                                }
                                std::cout << "shift was pressed\n";
                            }
                            break;
                        case ActionMode5_AddYTick:
                            if (app.isCtrlPressed() && YTicks.size() < 2)
                            {
                                float value = 0.0f;
#ifdef _DEBUG
                                
                                value = (float) YTicks.size();
							if (value > 0.5f)
							{
								value = YTicks[0].z + 1.0f;
							}
#endif // _DEBUG
                                
                                YTicks.push_back(ImVec4(HoveredPixel.x, HoveredPixel.y, value, 0.0f));
                                SelectedItem = YTicks.size() - 1;
                            }
                            else if (HoveredItem >= 0 && HoveredItem < int(YTicks.size()))
                            {
                                if (app.isShiftPressed())
                                {
                                    YTicks.erase(YTicks.begin() + HoveredItem);
                                    HoveredItem = -1;
                                    SelectedItem = -1;
                                }
                                else
                                {
                                    SelectedItem = HoveredItem;
                                }
                                std::cout << "shift was pressed\n";
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
                            if (SelectedItem >= 0)
                            {
                                UserPoints[SelectedItem] = HoveredPixel;
                                if (app.isCtrlPressed()) // snap only if we hold CTRL
                                {
                                    SnapToCurve(UserPoints[SelectedItem]);
                                    SnapToBary(UserPoints[SelectedItem]);
                                }
                                SortPoints();
                                UpdateSubdivision();
                            }
                            break;
                        case ActionMode2_MoveOrigin:
                            CoordOriginTargetX += HoveredPixel - CoordOriginImg;
                            CoordOriginImg = HoveredPixel;
                            
                            SnappedPoint = CoordOriginImg;
                            if (app.isCtrlPressed()) // snap only if we hold CTRL
                            {
                                SnapToCurve(SnappedPoint);
                                SnapToBary(SnappedPoint);
                                CoordOriginTargetX += SnappedPoint - CoordOriginImg;
                                CoordOriginImg = SnappedPoint;
                            }
                            break;
                        case ActionMode3_MoveXTarget:
                            SnappedPoint = HoveredPixel;
                            
                            if (app.isCtrlPressed()) // snap only if we hold CTRL
                            {
                                SnapToCurve(SnappedPoint);
                                SnapToBary(SnappedPoint);
                            }
                            CoordOriginTargetX = SnappedPoint;
                            
                            SortPoints();
                            UpdateSubdivision();
                            break;
                        case ActionMode4_AddXTick:
                            if (SelectedItem >= 0)
                            {
                                SnappedPoint = HoveredPixel;
                                
                                if (app.isCtrlPressed()) // snap only if we hold CTRL
                                {
                                    SnapToCurve(SnappedPoint);
                                    SnapToBary(SnappedPoint);
                                }
                                XTicks[SelectedItem].x = SnappedPoint.x;
                                XTicks[SelectedItem].y = SnappedPoint.y;
                            }
                            break;
                        case ActionMode5_AddYTick:
                            if (SelectedItem >= 0)
                            {
                                SnappedPoint = HoveredPixel;
                                
                                if (app.isCtrlPressed()) // snap only if we hold CTRL
                                {
                                    SnapToCurve(SnappedPoint);
                                    SnapToBary(SnappedPoint);
                                }
                                YTicks[SelectedItem].x = SnappedPoint.x;
                                YTicks[SelectedItem].y = SnappedPoint.y;
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
                    if (SelectedItem >= 0)
                    {
                        
                        if (image->isPixelInside((int)HoveredPixel.x, (int)HoveredPixel.y))
                        {
                            UserPoints[SelectedItem] = HoveredPixel;
                            
                            if (app.isCtrlPressed()) // snap only if we hold CTRL
                            {
                                SnapToCurve(UserPoints[SelectedItem]);
                                SnapToBary(UserPoints[SelectedItem]);
                            }
                            
                            SortPoints();
                            UpdateSubdivision();
                        }
                        
                        
                        SelectedItem = -1;
                    }
                    break;
                case ActionMode2_MoveOrigin:
                    break;
                case ActionMode4_AddXTick:
                    if (SelectedItem >= 0)
                    {
                        ImGui::OpenPopup("TickConfig");
                    }
                    break;
                case ActionMode5_AddYTick:
                    if (SelectedItem >= 0)
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
    
    if (std::abs(CoordOriginTargetX.x - CoordOriginImg.x) < 2.0f && std::abs(CoordOriginTargetX.y - CoordOriginImg.y) < 2.0f)
    {
        CoordOriginTargetX = CoordOriginImg + ImVec2(10.0f, 0.0f);
    }
    
    if (ImGui::IsMouseReleased(0) && !bIsContextMenuOpened)
    {
        bIsReadyForAction = true;
    }
    
    
}

void MainWindow::ShowPoints(float im_scale, ImVec2 im_pos, ImVec2 MousePos)
{
    ImVec2 WinPos = ImGui::GetWindowPos();
    
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    //draw lines between subdivided points
    if (SubdividedPoints.size() > 1)
    {
        for (size_t kp = 0; kp < SubdividedPoints.size() - 1; kp++)
        {
            ImVec2 PointPos0 = SubdividedPoints[kp] * im_scale + im_pos + WinPos;
            ImVec2 PointPos1 = SubdividedPoints[kp + 1] * im_scale + im_pos + WinPos;
            
            
            ImU32 LineColor = ImColor(80, 255, 80, 200);
            draw_list->AddLine(PointPos0, PointPos1, LineColor, 2.0f);
            
            if (bDrawSubdivideMarkers)
            {
                draw_list->AddCircleFilled(PointPos0, 4.0f, ImColor(80, 255, 80, 200), 8);
                draw_list->AddCircle(PointPos0, 5.0f, ImColor(0, 0, 0, 200), 8, 2.0f);
            }
        }
    }
    
    
    
    //if we are pressing button - draw a line from press location to snapped point
    if (SelectedItem >= 0 && ImGui::IsMouseDown(0) && CurrentMode == ActionMode1_AddPoints
        && SelectedItem < int(UserPoints.size()) && ImGui::IsMouseHoveringWindow())
    {
        ImVec2 PointPos0 = UserPoints[SelectedItem] * im_scale + im_pos + WinPos;
        
        ImU32 LineColor = ImColor(80, 220, 80, 220);
        draw_list->AddLine(PointPos0, MousePos, LineColor, 2.0f);
    }
    
    auto& app=MainApp::getInstance();
    
    for (size_t kp = 0; kp < UserPoints.size(); kp++)
    {
        ImVec2 PointPos = UserPoints[kp] * im_scale + im_pos + WinPos;
        
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
    static bool bTickInputAutoFocus = true;
    
    static bool bTickConfigInit = true;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10, 10));
    if (ImGui::BeginPopupModal("TickConfig", NULL, ImGuiWindowFlags_AlwaysAutoResize))
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
        static float TickValue = 0;
        
        if (bTickConfigInit)
        {
            if (CurrentMode == ActionMode4_AddXTick)//X
            {
                TickValue = XTicks[SelectedItem].z;
            }
            else if (CurrentMode == ActionMode5_AddYTick)//Y
            {
                TickValue = YTicks[SelectedItem].z;
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
        
        ImGui::InputFloat("Value", &TickValue, 0.0f, 0.0f, TickPrecision);
        
        auto& app=MainApp::getInstance();
        
        if (ImGui::Button("OK", ImVec2(120, 0)) || app.isEnterReleased())
        {
            //std::cout << TickValue << ", " << SelectedItem << '\n';
            
            if (CurrentMode == ActionMode4_AddXTick)//X
            {
                XTicks[SelectedItem].z = TickValue;
            }
            else if (CurrentMode == ActionMode5_AddYTick)//Y
            {
                YTicks[SelectedItem].z = TickValue;
            }
            
            SelectedItem = -1;
            bTickConfigInit = true;
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            
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
    ImVec2 WinPos = ImGui::GetWindowPos();
    
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    ImU32 TickColor = ImColor(120, 120, 120, 255);
    
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
        
        ImVec2 XTickPosition;
        
        XTickPosition.x = XTicks[kp].x;
        XTickPosition.y = XTicks[kp].y;
        
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
        ImGui::Text("%0.2f", XTicks[kp].z);
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
        
        
        ImVec2 YTickPosition;
        
        YTickPosition.x = YTicks[kp].x;
        YTickPosition.y = YTicks[kp].y;
        
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
        ImGui::Text("%0.2f", YTicks[kp].z);
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
    if(!image)
        return;
    
    ImVec2 WinPos = ImGui::GetWindowPos();
    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    float im_width = float(image->get_width())*CurrentImageScale;
    
    
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

void MainWindow::UpdateHoveredItemIndex(ImVec2 im_pos)
{
    ImVec2 WinPos = ImGui::GetWindowPos();
    ImVec2 MousePos = ImGui::GetMousePos();
    
    HoveredItem = -1;
    float MinDist = float(ZoomPixelHSide);
    float MinDist2 = MinDist*MinDist;
    
    if (CurrentMode == ActionMode1_AddPoints)
    {
        
        for (size_t kp = 0; kp < UserPoints.size(); kp++)
        {
            ImVec2 DeltaPos = UserPoints[kp] * CurrentImageScale + im_pos + WinPos - MousePos;
            
            float dist2 = DeltaPos.x*DeltaPos.x + DeltaPos.y*DeltaPos.y;
            
            if (dist2 < MinDist2)
            {
                MinDist2 = dist2;
                HoveredItem = kp;
            }
        }
    }
    else if (CurrentMode == ActionMode4_AddXTick)
    {
        for (size_t kp = 0; kp < XTicks.size(); kp++)
        {
            //float DeltaPos = std::abs(XTicks[kp].x * im_scale + im_pos.x + WinPos.x - MousePos.x);
            
            ImVec2 Direction;
            Direction.x = CoordOriginTargetX.y - CoordOriginImg.y;
            Direction.y = -(CoordOriginTargetX.x - CoordOriginImg.x);
            
            ImVec2 Point;
            Point.x = XTicks[kp].x;
            Point.y = XTicks[kp].y;
            
            float A = Direction.y;
            float B = -Direction.x;
            float C = -A*Point.x - B*Point.y;
            //	float C2 = B*Point.x - A*Point.y;
            
            
            ImVec2 MouseImg = (MousePos - WinPos - im_pos) / CurrentImageScale;
            
            float DeltaPos = (A*MouseImg.x + B*MouseImg.y + C);
            DeltaPos = DeltaPos*DeltaPos / (A*A + B*B);
            
            //std::cout << "del: " << DeltaPos << " mx:" << MouseImg.x << "," << MouseImg.y << "\n";
            
            if (DeltaPos < MinDist2)
            {
                MinDist2 = DeltaPos;
                HoveredItem = kp;
            }
        }
    }
    else if (CurrentMode == ActionMode5_AddYTick)
    {
        for (size_t kp = 0; kp < YTicks.size(); kp++)
        {
            ImVec2 Direction;
            Direction.x = CoordOriginTargetX.x - CoordOriginImg.x;
            Direction.y = CoordOriginTargetX.y - CoordOriginImg.y;
            
            
            
            ImVec2 Point;
            Point.x = YTicks[kp].x;
            Point.y = YTicks[kp].y;
            
            float A = Direction.y;
            float B = -Direction.x;
            float C = -A*Point.x - B*Point.y;
            
            
            ImVec2 MouseImg = (MousePos - WinPos - im_pos) / CurrentImageScale;
            
            float DeltaPos = (A*MouseImg.x + B*MouseImg.y + C);
            DeltaPos = DeltaPos*DeltaPos / (A*A + B*B);
            
            
            if (DeltaPos < MinDist)
            {
                MinDist = DeltaPos;
                HoveredItem = kp;
            }
        }
    }
    
    
}

void MainWindow::ShowSidePanel()
{
    float SettingsWidth = 250.0f;
    
    
    ImGui::BeginChild("SettingsWindow", ImVec2(SettingsWidth, 0));
    ImGui::Text("Settings");
    
    
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
    
    if (PrevSubdivideIterations <=0)
        ImGui_PushDisableButton();
    if (ImGui::Button("-", ImVec2(30.f, 0)) && SubdivideIterations>0)
    {
        SubdivideIterations--;
    }
    if (PrevSubdivideIterations <=0)
        ImGui_PopDisableButton();
    
    ImGui::SameLine();
    ImGui::SetCursorPosX(SettingsWidth - 30.0f);
    
    if (PrevSubdivideIterations >=MaxSubdivideIterations)
        ImGui_PushDisableButton();
    if (ImGui::Button("+", ImVec2(30.f, 0)) && SubdivideIterations<MaxSubdivideIterations)
    {
        SubdivideIterations++;
    }
    if (PrevSubdivideIterations >=MaxSubdivideIterations)
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
    float barWidth = (barTotalWidth - (MaxSubdivideIterations - 1)*barSpacing) / float(MaxSubdivideIterations);
    
    for (int i = 0; i < MaxSubdivideIterations; i++)
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
    
    
    if (image)
    {
        
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
        
        if (SubdivideIterations != PrevSubdivideIterations ||
            PrevBinarizationLevel != BinarizationLevel)
            UpdateSubdivision(true);
        
        
        
        
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
    bool bExportReady = IsReadyForExport(out_Result, true);
    
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
        ExportToClipboard();
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


void MainWindow::numToStr(float num, std::string& out_String)
{
    
    out_String = std::to_string(num);
    
    if (decimalSeparator == '.')
        std::replace(out_String.begin(), out_String.end(), ',', '.');
    else if (decimalSeparator == ',')
        std::replace(out_String.begin(), out_String.end(), '.', ',');
    
    //std::cout << out_String << "\n";
}

void MainWindow::ImVec2toString(const ImVec2& num, std::string& out_String)
{

}

void MainWindow::OpenImage()
{
    char const * filename;
    char const * lFilterPatterns[3] = { "*.png", "*.jpg", "*.bmp" };
    
    std::string path;
    
    auto full_path=std::filesystem::current_path();
    //std::cout << "Current path is : " << full_path << std::endl;
    
    path = full_path.string();
    
    path.append("\\");
    
    std::cout << "default path: " << path << "\n";
    
    path = "";
    
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
    
    ResetAll();
}

bool MainWindow::IsReadyForExport(int& out_Result, bool bSilent /*=false*/)
{
    
    out_Result = ExportReadyStatus::ExportReadyStatus_Ready;
    
    if (SortedUserPoints.size() < 2)
    {
        out_Result |= ExportReadyStatus::ExportReadyStatus_NoPoints;
    }
    if (XTicks.size() < 2)
    {
        out_Result |= ExportReadyStatus::ExportReadyStatus_NoXTicks;
    }
    else
    {
        float norm = std::sqrt((CoordOriginTargetX.x - CoordOriginImg.x)*(CoordOriginTargetX.x - CoordOriginImg.x) +
                               (CoordOriginTargetX.y - CoordOriginImg.y)*(CoordOriginTargetX.y - CoordOriginImg.y));
        
        //for deriving a and b
        float det1 = ((XTicks[0].x - XTicks[1].x)*(CoordOriginTargetX.x - CoordOriginImg.x)
                      + (XTicks[0].y - XTicks[1].y)*(CoordOriginTargetX.y - CoordOriginImg.y))/norm;
        
        //std::cout << "det1: "<< det1 << "\n";
        
        //float dist=det1/
        
        if (std::abs(det1) < MinTickPixelDistance)
        {
            out_Result |= ExportReadyStatus::ExportReadyStatus_XTicksSimilarPositions;
        }
        else if (std::abs(XTicks[0].z - XTicks[1].z) < MinTickRealDistance)
        {
            out_Result |= ExportReadyStatus::ExportReadyStatus_XTicksSimilarValues;
        }
    }
    
    if (YTicks.size() < 2)
    {
        out_Result |= ExportReadyStatus::ExportReadyStatus_NoYTicks;
    }
    else
    {
        float norm = std::sqrt((CoordOriginTargetX.x - CoordOriginImg.x)*(CoordOriginTargetX.x - CoordOriginImg.x) +
                               (CoordOriginTargetX.y - CoordOriginImg.y)*(CoordOriginTargetX.y - CoordOriginImg.y));
        
        //for deriving c and d
        float det2 = ((YTicks[0].x - YTicks[1].x)*(CoordOriginImg.y - CoordOriginTargetX.y)
                      - (YTicks[0].y - YTicks[1].y)*(CoordOriginImg.x - CoordOriginTargetX.x))/norm;
        
        //std::cout << "det2: " << det2 << "\n";
        
        if (std::abs(det2)<MinTickPixelDistance)
        {
            out_Result |= ExportReadyStatus::ExportReadyStatus_YTicksSimilarPositions;
        }
        else if (std::abs(YTicks[0].z - YTicks[1].z)<MinTickRealDistance)
        {
            out_Result |= ExportReadyStatus::ExportReadyStatus_YTicksSimilarValues;
        }
    }
    
    if (out_Result != ExportReadyStatus_Ready)
    {
        if(!bSilent)
        {
            if (out_Result & ExportReadyStatus_NoPoints)
            {
                tinyfd_messageBox(
                        "Error",
                        "At least two points should be defined!",
                        "ok",
                        "error",
                        1);
            }
            else if (out_Result & ExportReadyStatus_NoXTicks)
            {
                
                tinyfd_messageBox(
                        "Error",
                        "Not enough X ticks!",
                        "ok",
                        "error",
                        1);
            }
            else if (out_Result & ExportReadyStatus_NoYTicks)
            {
                
                tinyfd_messageBox(
                        "Error",
                        "Not enough Y ticks!",
                        "ok",
                        "error",
                        1);
            }
            else if (out_Result & ExportReadyStatus_XTicksSimilarPositions)
            {
                tinyfd_messageBox(
                        "Error",
                        "X ticks have overlapping positions!",
                        "ok",
                        "error",
                        1);
            }
            else if (out_Result & ExportReadyStatus_XTicksSimilarValues)
            {
                tinyfd_messageBox(
                        "Error",
                        "X ticks have overlapping values!",
                        "ok",
                        "error",
                        1);
            }
            else if (out_Result & ExportReadyStatus_YTicksSimilarPositions)
            {
                tinyfd_messageBox(
                        "Error",
                        "Y ticks have overlapping positions!",
                        "ok",
                        "error",
                        1);
            }
            else if (out_Result & ExportReadyStatus_YTicksSimilarValues)
            {
                tinyfd_messageBox(
                        "Error",
                        "Y ticks have overlapping values!",
                        "ok",
                        "error",
                        1);
            }
        }
        
        return false;
    }
    
    return true;
}

void MainWindow::ExportToClipboard()
{
    
    int out_Result;
    if (!IsReadyForExport(out_Result))
        return;
    
    SortPoints();
    UpdateSubdivision(true);
    SortArray(SubdividedPoints);
    
    ImVec2 RealPoint;
    std::stringstream sstr;
    //std::stringstream sstr;
    
    std::string nums = "";
    
    std::string tempColumnSeparator = columnSeparator;
    std::string tempLineEnding = lineEnding;
    
    
    //new line sequence that is needed by default notepad
    const char newLineSeq[3] = { char(0x0d), char(0x0a), '\0' };
    
    for (size_t i = 0; i<tempColumnSeparator.size(); i++)
    {
        if (tempColumnSeparator[i] == '\n')
        {
            tempColumnSeparator.replace(i, i + 1, newLineSeq);
            i++;//since we added one extra char
        }
    }
    
    for (size_t i = 0; i<tempLineEnding.size(); i++)
    {
        if (tempLineEnding[i] == '\n')
        {
            tempLineEnding.replace(i, i + 1, newLineSeq);
            i++;//since we added one extra char
        }
    }
    
    for (size_t kp = 0; kp < SubdividedPoints.size(); kp++)
    {
        RealPoint = ConvertImageToReal(SubdividedPoints[kp]);
        
        numToStr(RealPoint.x, nums);
        
        sstr << nums;
        sstr << tempColumnSeparator;
        
        numToStr(RealPoint.y, nums);
        sstr<< nums;
        sstr << tempLineEnding;
        //sstr << newLineSeq;
    }
    
    MainApp::getInstance().copy_to_clipboard(sstr.str().c_str());
}

void MainWindow::ExportPoints()
{
    char const * lFilterPatterns[2] = { "*.txt", "*.mat" };
    
    std::string path;
    
    
    auto full_path=std::filesystem::current_path();
    //std::cout << "Current path is : " << full_path << std::endl;
    
    path = full_path.string();
    
    path.append("\\");
    
    std::cout << "default path: " << path << "\n";
    
    path = "";
    
    
    path.append("data.txt");
    
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
    
    int out_Result;
    if (!IsReadyForExport(out_Result))
        return;
    
    
    
    SortPoints();
    UpdateSubdivision(true);
    SortArray(SubdividedPoints);
    
    ImVec2 RealPoint;
    
    std::vector<ImVec2> RealUserPoints = SortedUserPoints;
    std::vector<ImVec2> RealSubdividedPoints = SubdividedPoints;
    
    
    std::ofstream ofs;
    
    if(bUseTextFormat)
        ofs.open(lTheSaveFileName);// , std::ofstream::out | std::ofstream::app);
    
    
    
    
    for (size_t kp = 0; kp < SubdividedPoints.size(); kp++)
    {
        RealPoint = ConvertImageToReal(SubdividedPoints[kp]);
        RealSubdividedPoints[kp] = RealPoint;
        
        if (bUseTextFormat)
        {
            ofs << RealPoint.x << "\t" << RealPoint.y << "\n";
            
            //sstr << char(0x0d) << char(0x0a);
        }
        
    }
    
    if (bUseTextFormat)
    {
        ofs.close();
        return;
    }
    
    
    for (size_t kp = 0; kp < SortedUserPoints.size(); kp++)
    {
        RealPoint = ConvertImageToReal(SortedUserPoints[kp]);
        RealUserPoints[kp] = RealPoint;
    }
    
    
    FILE* fp=fopen(lTheSaveFileName, "wb");
    
    if (fp != nullptr)
    {
        writeHeader(fp);
        writeMatrixToMatFile(fp, "UserPointsPixels", &(SortedUserPoints[0].x), SortedUserPoints.size(), 2);
        writeMatrixToMatFile(fp, "SubdividedPointsPixels", &(SubdividedPoints[0].x), SubdividedPoints.size(), 2);
        writeMatrixToMatFile(fp, "UserPointsReal", &(RealUserPoints[0].x), RealUserPoints.size(), 2);
        writeMatrixToMatFile(fp, "SubdividedPointsReal", &(RealSubdividedPoints[0].x), RealSubdividedPoints.size(), 2);
    }
    
    fclose(fp);
}


void MainWindow::SortPoints()
{
    SortedUserPoints = UserPoints;
    
    SortArray(SortedUserPoints);
}


void MainWindow::SortArray(std::vector<ImVec2>& Array)
{
    struct sort_class_x
    {
        ImVec2 origin;
        ImVec2 target;
        
        bool operator() (ImVec2 i, ImVec2 j)
        {
            float proj_i = (target.x - origin.x)*(i.x - origin.x) + (target.y - origin.y)*(i.y - origin.y);
            float proj_j = (target.x - origin.x)*(j.x - origin.x) + (target.y - origin.y)*(j.y - origin.y);
            
            return proj_i<proj_j;
            //return (i.x<j.x);
        }
    } sort_objectX;
    
    sort_objectX.origin = CoordOriginImg;
    sort_objectX.target = CoordOriginTargetX;
    
    sort(Array.begin(), Array.end(), sort_objectX);
}

void MainWindow::ResetAll()
{
    UserPoints.clear();
    SortedUserPoints.clear();
    SubdividedPoints.clear();
    
    XTicks.clear();
    YTicks.clear();
    
    CoordOriginTargetX = CoordOriginImg + ImVec2(100.0f, 0.0f);
    
    CurrentImageScale = 0.0f;
    
    if (image)
    {
        
        CoordOriginImg = ImVec2((float)image->get_width(), (float)image->get_height())*0.5f;
        CoordOriginTargetX = CoordOriginImg + ImVec2((float)image->get_width()*0.3f, 0.0f);
    }
    
}

void MainWindow::UpdateSubdivision(bool bUpdateAll)
{
    int PointsNum = SortedUserPoints.size();
    
    if (PointsNum < 2)
    {
        SubdividedPoints.clear();
        return;
    }
    
    
    //extra points for each two user points
    //int ExtraPoints = std::pow(2, SubdivideIterations) - 1;
    int ExtraPoints = (1 << SubdivideIterations) - 1;//=(2^S)-1
    
    //std::cout << "ep: " << ExtraPoints << " new: " << (1 << SubdivideIterations)-1 << "\n";
    
    int SubPointNum = (ExtraPoints + 1)*(PointsNum - 1) + 1;
    
    
    static std::vector<ImVec2> LastUserPoints;
    
    
    if (SubdividedPoints.size() != SubPointNum)
        SubdividedPoints.resize(SubPointNum);
    
    
    
    //copy existing point to new array
    for (size_t k = 0; k < SortedUserPoints.size(); k++)
    {
        SubdividedPoints[k*(ExtraPoints+1)] = SortedUserPoints[k];
    }
    
    
    
    auto PrevPoints = SubdividedPoints;
    for (size_t k = 0; k < SortedUserPoints.size()-1; k++)
    {
        if (SortedUserPoints.size() == LastUserPoints.size())
        {
            if (!bUpdateAll
                && SortedUserPoints[k].x == LastUserPoints[k].x
                && SortedUserPoints[k].y == LastUserPoints[k].y
                && SortedUserPoints[k+1].x == LastUserPoints[k+1].x
                && SortedUserPoints[k+1].y == LastUserPoints[k+1].y)
            {
                continue;
            }
        }
        
        ImVec2 MidPoint = SortedUserPoints[k];
        for (int k_it = 0; k_it < SubdivideIterations; k_it++)
        {
            //k*(ExtraPoints + 1) - left, (k+1)*(ExtraPoints+1) - right
            
            int step = (1 << (SubdivideIterations-k_it));// for 0 it
            
            for (int k_p = 0; k_p < ExtraPoints + 1; k_p+=step)
            {
                int n0 = k*(ExtraPoints + 1) + k_p;
                
                MidPoint = (SubdividedPoints[n0] + SubdividedPoints[n0 + step]) * 0.5f;
                
                SnapToCurve(MidPoint);
                SnapToBary(MidPoint);
                SubdividedPoints[n0 + step / 2] = MidPoint;
            }
        }
        
        
        //if(!bSmoothPoints)
        continue;
        
        //std::cout << "---------\n";
        
        PrevPoints = SubdividedPoints;
        
        for (int k_p = 0; k_p < ExtraPoints; k_p++)
        {
            //float a = 1.0f - float(k_p + 1) / float(ExtraPoints + 1);
            //float a = 1.0f - 1.0f / float(ExtraPoints - k_p + 1);
            
            //std::cout << "a: " << a << "\n";
            
            MidPoint = (PrevPoints[k*(ExtraPoints + 1) + k_p] + PrevPoints[k*(ExtraPoints + 1) + k_p + 1]
                        + PrevPoints[k*(ExtraPoints + 1) + k_p + 2])/3.0f;
            
            //SnapToCurve(MidPoint);
            //SnapToBary(MidPoint);
            SubdividedPoints[k*(ExtraPoints + 1) + k_p + 1] = MidPoint;
        }
        //SubdividedPoints = PrevPoints;
    }
    
    
    //SortArray(SubdividedPoints);


// 	std::cout << "------------\n";
// 	for (auto it = SubdividedPoints.rbegin(); it != SubdividedPoints.rend(); ++it)
// 	{
// 		std::cout << "point (" << (*it).x << "," << (*it).y << ")\n";
// 	}
// 	std::cout << "------------\n";
    
    LastUserPoints = SortedUserPoints;
}

void MainWindow::SnapToCurve(ImVec2& ImagePoint)
{
    
    int localX, localY;//to this vars local coords will be written
    
    int hside = ZoomPixelHSide;
    
    localX = int(ImagePoint.x);
    localY = int(ImagePoint.y);
    
    if (!image->getClosestBlack(localX, localY, hside, BinarizationLevel))
    {
        //std::cout << "Bad point!\n";
        return;
    }
    
    if (localX != int(ImagePoint.x))
    {
        ImagePoint.x = float(localX);
    }
    if (localY != int(ImagePoint.y))
    {
        ImagePoint.y = float(localY);
    }
    //std::cout << "loc snap: (" << snapX << "," << snapY << ")\n";
}

void MainWindow::SnapToBary(ImVec2& ImagePoint)
{
    int localX, localY;//to this vars local coords will be written
    
    int baryX, baryY; //local coords of snapped point
    
    int hside = 3;// ZoomPixelHSide;
    
    localX = int(ImagePoint.x);
    localY = int(ImagePoint.y);
    
    MatrixXi PointRegion;
    
    if (!image->getNearbyPoints(localX, localY, hside, PointRegion))
    {
        std::cout << "Bad point!\n";
        return;
    }
    
    //by default just leave point where it was
    baryX = localX;
    baryY = localY;
    
    
    
    int baryMass = 0;
    
    ImVec2 BaryOffset;
    
    for (int kx = 0; kx <= 2 * hside; kx++)
    {
        for (int ky = 0; ky <= 2 * hside; ky++)
        {
            if (PointRegion(ky, kx) < BinarizationLevel)
            {
                baryMass += BinarizationLevel - PointRegion(ky, kx);
                
                BaryOffset += ImVec2(float(kx - localX), float(ky - localY))*float(BinarizationLevel - PointRegion(ky, kx));
                
            }
            
        }
    }
    
    if (baryMass > 0)
    {
        BaryOffset /= float(baryMass);
        
        ImagePoint += BaryOffset;
    }
    
    //ImagePoint.x += (baryX - localX);
    //ImagePoint.y += (baryY - localY);
}

ImVec2 MainWindow::ConvertImageToReal(const ImVec2& ImagePoint)
{
    ImVec2 RealPoint;
    
    //this function should be called after check that
    //we can really calculate real points
    //so all x and y ticks are defined
    
    ImVec2 Scale, Offset;
    
    Scale.x = (XTicks[0].z - XTicks[1].z) / (XTicks[0].x - XTicks[1].x);
    Scale.y = (YTicks[0].z - YTicks[1].z) / (YTicks[0].y - YTicks[1].y);
    
    
    Offset.x = XTicks[1].z - XTicks[1].x*Scale.x;
    Offset.y = YTicks[1].z - YTicks[1].y*Scale.y;
    
    //XTicks[1].z + (ImagePoint.x- XTicks[1].x)*Scale.x
    
    RealPoint.x = Offset.x + ImagePoint.x*Scale.x;
    RealPoint.y = Offset.y + ImagePoint.y*Scale.y;
    
    //for deriving a and b
    float det1 = (XTicks[0].x - XTicks[1].x)*(CoordOriginTargetX.x - CoordOriginImg.x)
                 + (XTicks[0].y - XTicks[1].y)*(CoordOriginTargetX.y - CoordOriginImg.y);
    
    //for deriving c and d
    float det2 = (YTicks[0].x - YTicks[1].x)*(CoordOriginImg.y - CoordOriginTargetX.y)
                 - (YTicks[0].y - YTicks[1].y)*(CoordOriginImg.x - CoordOriginTargetX.x);
    
    float a, b, c, d, e, f;
    
    a = (XTicks[0].z - XTicks[1].z)*(CoordOriginTargetX.x - CoordOriginImg.x) / det1;
    b = (XTicks[0].z - XTicks[1].z)*(CoordOriginTargetX.y - CoordOriginImg.y) / det1;
    
    c = (YTicks[0].z - YTicks[1].z)*(CoordOriginImg.y - CoordOriginTargetX.y) / det2;
    d = -(YTicks[0].z - YTicks[1].z)*(CoordOriginImg.x - CoordOriginTargetX.x) / det2;
    
    e = XTicks[0].z - a*XTicks[0].x - b*XTicks[0].y;
    f = YTicks[0].z - c*YTicks[0].x - d*YTicks[0].y;
    
    RealPoint.x = a*ImagePoint.x + b*ImagePoint.y + e;
    RealPoint.y = c*ImagePoint.x + d*ImagePoint.y + f;
    
    
    return RealPoint;
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

