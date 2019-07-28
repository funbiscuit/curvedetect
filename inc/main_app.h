#ifndef CURVEDETECT_MAIN_APP_H
#define CURVEDETECT_MAIN_APP_H

#include <imgui.h>
#include "main_window.h"

#include <vector>
#include <imgui.h>


enum ActionMode
{
    ActionMode0_None,
    ActionMode1_AddPoints,
    ActionMode2_MoveOrigin,
    ActionMode3_MoveXTarget,
    ActionMode4_AddXTick,
    ActionMode5_AddYTick,
    
};


enum ExportReadyStatus //: int
{
    ExportReadyStatus_Ready = 0,
    ExportReadyStatus_NoPoints = 1,
    ExportReadyStatus_NoXTicks = 2,
    ExportReadyStatus_NoYTicks = 4,
    ExportReadyStatus_XTicksSimilarPositions = 8,
    ExportReadyStatus_XTicksSimilarValues = 16,
    ExportReadyStatus_YTicksSimilarPositions = 32,
    ExportReadyStatus_YTicksSimilarValues = 64,
    
};


class GLFWwindow;

class MainApp
{
private:
    MainApp(const MainApp&);
    MainApp& operator=(MainApp&);
    MainApp();

public:
    
    bool init(GLFWwindow* window, const char* glsl_version);
    
    void new_frame();
    
    static MainApp& getInstance();

private:
    
    float MinTickPixelDistance = 5.0f;
    float MinTickRealDistance = 0.001f;
    
    ImVec4 ColorDisabled = ImVec4(0.78f, 0.78f, 0.78f, 1.0f);
    
    float ZoomWndSize;
    
    bool bCtrlPressed;
    bool bShiftPressed;
    bool bAltPressed;
    bool bEnterReleased;
    
    MainWindow mainWindow;
    
    static void onWindowResize(GLFWwindow* wnd, int width, int height);
    static void onKeyCallback(GLFWwindow* wnd, int key, int scancode, int action, int mods);
    
    bool bShouldUseIMGUICursor;
    
    int m_width, m_height;
    
    GLFWwindow *window;
    
    void SetUseIMGUICursor(bool bShowCursor);
    
    ActionMode CurrentMode;
    
    int ZoomPixelHSide;
    
    int SubdivideIterations;
    int MaxSubdivideIterations=6;
    
    char decimalSeparator;
    
    std::string columnSeparator;
    std::string lineEnding;
    
    int BinarizationLevel;
    
    bool bSmoothPoints;
    
    bool bDrawSubdivideMarkers = true;
    bool bShowImage = true;
    bool bShowBinarization = false;
    
    std::shared_ptr<Image> image;
    
    int SelectedItem;
    int HoveredItem;
    
    float MinImageScale;
    float CurrentImageScale;
    float MaxImageScale;
    
    
    ImVec2 CurrentImPos;
    
    bool bIsContextMenuOpened = false;
    bool bIsReadyForAction = true;
    
    std::vector<ImVec2> UserPoints; //position of user points (pixels)
    std::vector<ImVec2> SortedUserPoints; //position of user points (pixels)
    std::vector<ImVec2> SubdividedPoints; //position of all points (pixels) both user and generated
    
    
    std::vector<ImVec4> XTicks;	//first two - image coordinates, third - real X value, fourth - not used
    std::vector<ImVec4> YTicks;	//first two - image coordinates, third - real X value, fourth - not used
    
    ImVec2 CoordOriginImg; // position of origin in image (pixels)
    ImVec2 CoordOriginTargetX; // position of target in image (pixels) where X is pointed
    
    
    void numToStr(float num, std::string& out_String);
    
    
    void ImVec2toString(const ImVec2& num, std::string& out_String);
    
    void OpenImage();
    
    void ExportPoints();
    
    void ExportToClipboard();
    
    void ShowMainWindow();
    
    void ProcessInput(ImVec2 &HoveredPixel);
    
    void ShowPoints(float ImageScale, ImVec2 im_pos, ImVec2 MousePos);
    
    void ShowTickConfigPopup();
    
    bool ShowZoomWindow(const ImVec2 &canvas_sz, const ImVec2 &HoveredPixel, ImVec2& out_ZoomOrigin);
    
    void ShowTickLines(ImVec2 im_pos);
    
    void ShowCoordSystem(const ImVec2 &im_pos);
    
    void UpdateHoveredItemIndex(ImVec2 im_pos);
    
    void ShowSidePanel();
    
    void ImGui_PopDisableButton();
    
    void ImGui_PushDisableButton();
    
    void SortPoints();
    
    void SortArray(std::vector<ImVec2>& Array);
    
    void ResetAll();
    
    bool IsReadyForExport(int& out_Result, bool bSilent = false);
    
    void UpdateSubdivision(bool bUpdateAll = false);
    
    //will snap point to closest black (0) point
    //so for more precision use SnapToBary also
    void SnapToCurve(ImVec2& ImagePoint);
    
    //will snap point to barycenter of current zoom region
    //will not be precise if there are grid lines in the region
    //so make sure that in zoom region only curve points are shown
    void SnapToBary(ImVec2& ImagePoint);
    
    ImVec2 ConvertImageToReal(const ImVec2& ImagePoint); //will convert pixel coordinates to real data
    
    
    //tries to make a full line inside of specified region
    //that goes through specified point and points at specified direction
    //outputs result to out_Start and out_End
    bool MakeFullLine(ImVec2 Point, ImVec2 Direction, ImVec2& out_Start, ImVec2& out_End, ImVec2 RegionSize, ImVec2 RegionTL=ImVec2(0.0f,0.0f));
    
    
    
    std::string unescape(const std::string& s);
    std::string escape(const std::string& s);
    
};


#endif //CURVEDETECT_MAIN_APP_H
