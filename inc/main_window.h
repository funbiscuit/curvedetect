
#ifndef CURVEDETECT_MAINWINDOW_H
#define CURVEDETECT_MAINWINDOW_H

#include <memory>
#include "image.h"
#include "curve_detect.h"
#include "imgui.h"


enum ActionMode
{
    MODE_NONE    = 1 << 0,
    MODE_POINTS  = 1 << 1,
    MODE_HORIZON = 1 << 2,
    MODE_XTICKS  = 1 << 3,
    MODE_YTICKS  = 1 << 4,

    MODE_TICKS  = MODE_XTICKS | MODE_YTICKS

};


class MainWindow
{
public:
    int width;
    int height;
    MainWindow();

    void on_render();
    void on_resize(int w, int h);
    
    void init();
    
private:
    float toolbar_width;
    std::shared_ptr<Image> image;
    
    
    ImVec4 ColorDisabled = ImVec4(0.78f, 0.78f, 0.78f, 1.0f);
    
    float ZoomWndSize;
    
    
    ActionMode CurrentMode;
    
    int ZoomPixelHSide;
    
    
    char decimalSeparator;
    
    std::string columnSeparator;
    std::string lineEnding;
    
    
    bool bSmoothPoints;
    
    bool bDrawSubdivideMarkers = true;
    bool bShowImage = true;
    bool bShowBinarization = false;
    
    
    
    float MinImageScale;
    float CurrentImageScale;
    float MaxImageScale;
    
    
    ImVec2 CurrentImPos;
    ImVec2 HoveredPixel;

    bool bIsContextMenuOpened = false;
    bool bIsReadyForAction = true;

    bool deleteOnRelease = false;
    
    std::shared_ptr<CurveDetect> curve;
    
    void ShowMainWindow();
    void ShowImage(ImVec2 canvasSize);

    void ProcessInput();
    void OnMouseDown(int btn);
    void OnMouseUp(int btn);
    void OnMouseDrag(int btn);

    void ShowPoints(float ImageScale, ImVec2 im_pos, ImVec2 MousePos);
    
    void ShowTickConfigPopup();
    
    bool ShowZoomWindow(const ImVec2 &canvas_sz, ImVec2& out_ZoomOrigin);
    
    void ShowTickLines(ImVec2 im_pos);
    
    void ShowCoordSystem(const ImVec2 &im_pos);
    
    
    void ShowSidePanel();
    
    
    void OpenImage();
    
    
    void ImGui_PopDisableButton();
    
    void ImGui_PushDisableButton();
    
    
    void ResetAll();
    
    void ExportPoints();
    
    //tries to make a full line inside of specified region
    //that goes through specified point and points at specified direction
    //outputs result to out_Start and out_End
    bool MakeFullLine(ImVec2 Point, ImVec2 Direction, ImVec2& out_Start, ImVec2& out_End, ImVec2 RegionSize, ImVec2 RegionTL=ImVec2(0.0f,0.0f));
    
    
    
    std::string unescape(const std::string& s);
    std::string escape(const std::string& s);
    
};


#endif //CURVEDETECT_MAINWINDOW_H
