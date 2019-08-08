
#ifndef CURVEDETECT_MAINWINDOW_H
#define CURVEDETECT_MAINWINDOW_H

#include <memory>
#include "image.h"
#include "curve_detect.h"
#include "imgui.h"


enum ActionMode
{
    MODE_POINTS  = 1 << 1,
    MODE_HORIZON = 1 << 2,
    MODE_GRID  = 1 << 3,

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
    std::shared_ptr<CurveDetect> curve;


    ImColor colorDisabled = ImColor(200, 200, 200);

    float zoomWindowSize;
    int curveThickness=3;
    const int curveThicknessMin=2;
    const int curveThicknessMax=30;
    int subdivLevel=10;
    const int maxSubdivLevel=10;
    int binLevel=127;


    ActionMode currentMode;

    int zoomPixelHalfSide;


    char decimalSeparator;

    std::string columnSeparator;
    std::string lineEnding;


    bool bShowSubdivPoints = true;
    bool bShowImage = true;
    bool bShowBinarization = false;
    bool bInvertImage = false;
    bool bShowZoomWindow = false;
    bool bShowFps = false;



    float minImageScale;
    float imageScale;
    float maxImageScale;


    ImVec2 imagePosition;
    ImVec2 hoveredImagePixel;

    bool bIsContextMenuOpened = false;
    bool bIsReadyForAction = true;

    bool deleteOnRelease = false;


    void render_main_window();
    void render_image(ImVec2 canvasSize);

    void process_input();
    void on_mouse_down(int btn);
    void on_mouse_up(int btn);
    void on_mouse_double_click(int btn);
    void on_mouse_drag(int btn);

    void render_points(float ImageScale, ImVec2 im_pos, ImVec2 MousePos);

    void render_tick_config_popup();

    bool render_zoom_window(const ImVec2 &canvas_sz, ImVec2 &out_ZoomOrigin);

    void render_grid_lines(ImVec2 im_pos);

    void render_horizon(const ImVec2 &im_pos);


    void render_side_panel();
    void render_hints_panel();


    void on_open_image();
    void on_paste_image();


    void ImGui_PopDisableButton();

    void ImGui_PushDisableButton();


    void reset_all();

    void on_export_points();

    //tries to make a full line inside of specified region
    //that goes through specified point and points at specified direction
    //outputs result to out_Start and out_End
    bool extend_line(ImVec2 Point, ImVec2 Direction, ImVec2 &out_Start, ImVec2 &out_End, ImVec2 RegionSize,
                     ImVec2 RegionTL = ImVec2(0.0f, 0.0f));



    std::string unescape(const std::string& s);
    std::string escape(const std::string& s);

};


#endif //CURVEDETECT_MAINWINDOW_H
