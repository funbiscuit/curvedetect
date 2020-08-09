
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <fstream>

#include <QtWidgets>

#include "portable-file-dialogs.h"

#include "mainwindow.h"
#include "curveview.h"
//#include "main_app.h"
//#include "clipboard.h"



MainWindow::MainWindow()
{
    setMinimumSize(600, 400);

    readSettings();

    auto mainLayout = new QHBoxLayout;

    auto window = new QWidget();
    window->setLayout(mainLayout);
    setCentralWidget(window);

    createSidePanel();
    mainLayout->addLayout(sidePanelLayout);

    curveView = new CurveView();
    mainLayout->addWidget(curveView, 1);

    currentMode = ActionMode::MODE_POINTS;


    minImageScale = 1.0f;
    imageScale = 1.0f;

    decimalSeparator = '.';
    columnSeparator = "\t";
    lineEnding = "\n";

    image = nullptr;
    curve = nullptr;


    //TODO should be in curve view?
#ifndef NDEBUG
    image=std::make_shared<Image>("../../img/test.png");
    std::cout << "im " <<image->is_loaded()<<"\n";
    curve=std::make_shared<CurveDetect>(image);
    curve->reset_all();
    curveView->setCurve(curve);

    curve->add_point(Vec2D(168,3));
    curve->add_point(Vec2D(501,234));
    curve->add_point(Vec2D(765,505));
    curve->deselect_all();
#endif
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void MainWindow::createSidePanel()
{

//    QGroupBox* viewSettings = nullptr;
//    QGroupBox* curveSettings = nullptr;
//    QGroupBox* fileGroup = nullptr;

    sidePanelLayout = new QVBoxLayout();

    fileGroup = new QGroupBox("Open/Export");
    auto fileLayout = new QGridLayout();
    fileGroup->setLayout(fileLayout);

    auto openBtn = new QPushButton("Open");
    auto pasteBtn = new QPushButton("Paste");
    auto columnSep = new QLineEdit("\\t");
    auto lineEnd = new QLineEdit("\\n");
    auto decimalSep = new QComboBox();
    decimalSep->insertItem(0, "dot");
    decimalSep->insertItem(1, "comma");
    auto copyBtn = new QPushButton("Copy");
    auto exportBtn = new QPushButton("Export");

    //TODO prettify layout
    fileLayout->addWidget(openBtn,0,0);
    fileLayout->addWidget(pasteBtn,0,1);
    fileLayout->addWidget(new QLabel("Text export settings"), 1,0,1,2);
    fileLayout->addWidget(new QLabel("Column separator"), 2,0);
    fileLayout->addWidget(columnSep, 2,1);
    fileLayout->addWidget(new QLabel("Line ending"), 3,0);
    fileLayout->addWidget(lineEnd, 3,1);
    fileLayout->addWidget(new QLabel("Decimal separator"), 4,0);
    fileLayout->addWidget(decimalSep, 4,1);
    fileLayout->addWidget(copyBtn,5,0);
    fileLayout->addWidget(exportBtn,5,1);


    // view settings
    viewSettings = new QGroupBox("View");
    auto viewLayout = new QGridLayout();
    viewSettings->setLayout(viewLayout);

    auto subdivCheck = new QCheckBox("Subdivision");
    auto zoomCheck = new QCheckBox("Zoom");
    auto imageCheck = new QCheckBox("Image");
    auto binCheck = new QCheckBox("Binarization");
    auto majGridCheck = new QCheckBox("Major grid");
    auto minGridCheck = new QCheckBox("Minor grid");

    viewLayout->addWidget(subdivCheck,0,0);
    viewLayout->addWidget(zoomCheck,0,1);
    viewLayout->addWidget(imageCheck,1,0);
    viewLayout->addWidget(binCheck,1,1);
    viewLayout->addWidget(majGridCheck,2,0);
    viewLayout->addWidget(minGridCheck,2,1);


    // curve settings
    curveSettings = new QGroupBox("Curve");
    auto curveLayout = new QGridLayout();
    curveSettings->setLayout(curveLayout);

    auto subdivLabel = new QLabel("Subdivision");
    auto subdivSlider = new QSlider(Qt::Horizontal);
    subdivSlider->setMaximum(10);
    subdivSlider->setValue(5);

    auto threshLabel = new QLabel("Threshold");
    auto threshSlider = new QSlider(Qt::Horizontal);
    threshSlider->setMaximum(255);
    threshSlider->setValue(127);

    auto thickLabel = new QLabel("Thickness");
    auto thickSlider = new QSlider(Qt::Horizontal);
    thickSlider->setMaximum(30);
    thickSlider->setMinimum(2);
    thickSlider->setValue(3);

    auto xScaleCB = new QComboBox();
    xScaleCB->insertItem(0, "linear X");
    xScaleCB->insertItem(1, "log X");
    auto yScaleCB = new QComboBox();
    yScaleCB->insertItem(0, "linear Y");
    yScaleCB->insertItem(1, "log Y");

    auto invertCheck = new QCheckBox("Invert Image");
    auto resetBtn = new QPushButton("Reset");

    curveLayout->addWidget(subdivLabel,0,0);
    curveLayout->addWidget(subdivSlider,0,1);
    curveLayout->addWidget(threshLabel,1,0);
    curveLayout->addWidget(threshSlider,1,1);
    curveLayout->addWidget(thickLabel,2,0);
    curveLayout->addWidget(thickSlider,2,1);
    curveLayout->addWidget(xScaleCB,3,0);
    curveLayout->addWidget(yScaleCB,3,1);
    curveLayout->addWidget(invertCheck,4,0);
    curveLayout->addWidget(resetBtn,4,1);


    auto helpArea = new QTextEdit();
    helpArea->setText("Open or paste new image");
    helpArea->setReadOnly(true);

    sidePanelLayout->addWidget(viewSettings,0);
    sidePanelLayout->addWidget(curveSettings,0);
    sidePanelLayout->addWidget(fileGroup,0);
    sidePanelLayout->addWidget(helpArea,1);
}

void MainWindow::readSettings()
{
    QSettings settings;
    const QByteArray geometry = settings.value(SETTINGS_GEOMETRY, QByteArray()).toByteArray();
    if (geometry.isEmpty())
    {
        const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else
        restoreGeometry(geometry);
}

void MainWindow::writeSettings()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
}

void MainWindow::on_render()
{



//    ImGui::SetNextWindowPos(ImVec2(0, 0));
//    ImGui::SetNextWindowSize(ImVec2((float)width, (float)height));// , ImGuiSetCond_FirstUseEver);
//
//    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
//    if (ImGui::Begin("Main Window", nullptr, window_flags))
//    {
//        render_main_window();
//        ImGui::End();
//    }
//    ImGui::PopStyleVar();

}

void MainWindow::init(float _fontScale)
{
#ifndef NDEBUG
//    image=std::make_shared<Image>("../img/test.png");
//    curve=std::make_shared<CurveDetect>(image);
//    curve->reset_all();
#endif
    fontScale = _fontScale;
}

void MainWindow::on_resize(int w, int h)
{

}

void MainWindow::render_main_window()
{
    render_side_panel();

    /*ImGui::SameLine();


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
    ImGui::EndGroup();*/
}

/*void MainWindow::render_image(ImVec2 canvasSize)
{
    if(image)
    {
        auto im_width = (float) image->get_width();
        auto im_height = (float) image->get_height();

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
}*/

void MainWindow::on_mouse_double_click(int btn)
{
//    auto& app= MainApp::get();
    if (btn == 0)
    {
        switch (currentMode)
        {
            case ActionMode::MODE_GRID:
//                if(curve->get_selected_id())
//                    ImGui::OpenPopup("TickConfig");
                break;
            default:
                break;
        }
    }
}


void MainWindow::process_input()
{
    /*ImGuiIO& io = ImGui::GetIO();
    auto& app= MainApp::get();
    static bool bIsMouseDownFirst = true;
    static ImVec2 lastMousePos = ImGui::GetMousePos();
    static bool prevCtrl = io.KeyCtrl;
    ImVec2 delta = ImGui::GetMousePos()-lastMousePos;
    bool moving = (std::abs(delta.x)+std::abs(delta.y))>0.f || prevCtrl != io.KeyCtrl;


    if (ImGui::IsWindowFocused())
    {
        ActionMode modes[]={MODE_POINTS,MODE_GRID,MODE_HORIZON};

        for(int j=0;j<3;++j)
            if (io.KeysDown[GLFW_KEY_1+j] || io.KeysDown[GLFW_KEY_KP_1+j])
                currentMode = modes[j];

        if(io.KeysDown[GLFW_KEY_F] && io.KeysDownDurationPrev[GLFW_KEY_F]==0.f && io.KeyCtrl)
            bShowFps = !bShowFps;

        if(curve && curve->get_selected_id()==0)
            deleteOnRelease = io.KeyShift;
    }

    if (curve && ImGui::is_mouse_hovering_window() && !bIsContextMenuOpened && bIsReadyForAction)
    {
        if (ImGui::IsMouseDown(0) && image->is_pixel_inside((int) hoveredImagePixel.x, (int) hoveredImagePixel.y))
        {
            bIsMouseDownFirst = false;
        }
        if (ImGui::IsMouseReleased(0))
        {
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
    prevCtrl = io.KeyCtrl;*/
}

/*
int onTickInput(ImGuiInputTextCallbackData *data)
{
    std::string tickVal = data->Buf;
    ImageTickLine::filter_value(tickVal, false);
    strcpy(data->Buf,tickVal.c_str());
    data->BufTextLen=(int)tickVal.length();
    if(data->CursorPos>(int)tickVal.length())
        data->CursorPos=(int)tickVal.length();
    data->BufDirty=true;
    return 0;
}*/

void MainWindow::render_tick_config_popup()
{
//    if(!curve)
//        return;

    static bool bTickInputAutoFocus = true;

    static bool bTickConfigInit = true;

    /*ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f);
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
    ImGui::PopStyleVar(2);*/
}
/*
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


}*/

void MainWindow::render_side_panel()
{
    float SettingsWidth = fontScale*250.0f;
    float columnMargin = fontScale*3.f;
    float secondColumnX = SettingsWidth/2+columnMargin;
/*
    static bool scrollbarShown = false;
    auto size = ImVec2(SettingsWidth, 0.f);
    if(scrollbarShown)
        size.x+=22.f;
    ImGui::BeginChild("SettingsWindow", size);
    scrollbarShown =ImGui::GetScrollMaxY()>0;
    auto pos = ImGui::GetCursorScreenPos();



    if (curve)
    {
        bool prevShowSubTicks=bShowSubTicks;
        ImGui::TextUnformatted("Visualisation settings");
        ImGui::Checkbox("Subdivision", &bShowSubdivPoints);
        ImGui::SameLine(secondColumnX);
        ImGui::Checkbox("Zoom", &bShowZoomWindow);
        ImGui::Checkbox("Image", &bShowImage);
        ImGui::SameLine(secondColumnX);
        ImGui::Checkbox("Binarization", &bShowBinarization);
        ImGui::Checkbox("Major grid", &bShowTicks);
        ImGui::SameLine(secondColumnX);
        ImGui::Checkbox("Minor grid", &bShowSubTicks);
        ImGui::PushClipRect(pos,ImVec2(pos.x+SettingsWidth,(float)height),true);
        ImGui::Separator();
        ImGui::PopClipRect();

        if(bShowSubTicks==prevShowSubTicks)
            bShowSubTicks = bShowSubTicks && bShowTicks;
        bShowTicks |= bShowSubTicks;

        ImGui::TextUnformatted("Curve settings");

        ImVec2 CurPos = ImGui::GetCursorPos();
        ImGui::SetCursorPosY(CurPos.y + fontScale*3.0f);
        ImGui::Text("Subdivision:");
        ImGui::SameLine(secondColumnX);
        ImGui::SetCursorPosY(CurPos.y);

        ImGui::PushItemWidth(SettingsWidth - secondColumnX);
        ImGui::SliderInt("##subdiv_slider", &subdivLevel, 0, maxSubdivLevel);
        ImGui::PopItemWidth();


        CurPos = ImGui::GetCursorPos();

        ImGui::SetCursorPosY(CurPos.y + fontScale*3.0f);
        ImGui::Text("Threshold: ");
        ImGui::SameLine(secondColumnX);
        ImGui::SetCursorPosY(CurPos.y);

        ImGui::PushItemWidth(SettingsWidth - secondColumnX);
        ImGui::SliderInt("##bin-level", &binLevel, 0, 255);

        ImGui::PopItemWidth();


        ImGui_ImplOpenGL3_SetBinarizationLevel(binLevel);
        ImGui_ImplOpenGL3_SetInvertImage(bInvertImage);

        curve->set_subdiv_level(subdivLevel);
        curve->set_bin_level(binLevel);
        curve->set_invert_image(bInvertImage);



        //tune curve thickness
        CurPos = ImGui::GetCursorPos();

        ImGui::SetCursorPosY(CurPos.y + fontScale*3.0f);
        ImGui::Text("Thickness: ");
        ImGui::SameLine(secondColumnX);
        ImGui::SetCursorPosY(CurPos.y);
        ImGui::PushItemWidth(SettingsWidth - secondColumnX);
        ImGui::SliderInt("##curve-thick", &curveThickness, curveThicknessMin, curveThicknessMax);
        ImGui::PopItemWidth();
        curve->set_curve_thickness(curveThickness);



        float scale_width=SettingsWidth-secondColumnX;
        ImGui::PushItemWidth(scale_width);
        const char* scalesX[] = { "linear X", "log X" };
        const char* scalesY[] = { "linear Y", "log Y" };
        const CurveDetect::AxisScale scales_enum[] ={
                CurveDetect::LINEAR, CurveDetect::LOG };
        static int xscale = 0;
        static int yscale = 0;
        ImGui::Combo("##xscale", &xscale, scalesX, 2);
        ImGui::SameLine(secondColumnX);
        ImGui::Combo("##yscale", &yscale, scalesY, 2);
        bLogX=xscale==1;
        bLogY=yscale==1;

        curve->set_scales(scales_enum[xscale], scales_enum[yscale]);

        ImGui::Checkbox("Invert Image", &bInvertImage);
        ImGui::SameLine(secondColumnX);

        bool resetPossible = curve->can_reset();

        if(!resetPossible)
            ImGui_PushDisableButton();
        if (ImGui::Button("Reset", ImVec2(SettingsWidth-secondColumnX, 0)) && resetPossible)
            on_reset();
        if(!resetPossible)
            ImGui_PopDisableButton();
        ImGui::PushClipRect(pos,ImVec2(pos.x+SettingsWidth,(float)height),true);
        ImGui::Separator();
        ImGui::PopClipRect();
    }

    if (ImGui::Button("Open", ImVec2(SettingsWidth/2-columnMargin, 0)))
        on_open_image();

    ImGui::SameLine(0.f,columnMargin*2);

    if (ImGui::Button("Paste", ImVec2(SettingsWidth/2-columnMargin, 0)))
        on_paste_image();

    if (curve)
    {
        ImGui::PushClipRect(pos,ImVec2(pos.x+SettingsWidth,(float)height),true);
        ImGui::Separator();
        ImGui::PopClipRect();
        ImGui::TextUnformatted("Text export settings");

        //render text export settings

        int out_Result;
        bool bExportReady = curve ? curve->is_export_ready(out_Result) : false;

        static char edit_buf_col_sep[10];
        static char edit_buf_line_end[10];

        std::string edit_str;

        edit_str = escape(columnSeparator);
        edit_str.copy(edit_buf_col_sep, 8);

        float inputWidth=SettingsWidth-secondColumnX;

        ImGui::PushItemWidth(inputWidth);
        ImGui::TextUnformatted("Column separator");
        ImGui::SameLine(secondColumnX);
        ImGui::InputText("##col-sep", edit_buf_col_sep, 6);

        edit_str = edit_buf_col_sep;
        columnSeparator = unescape(edit_str);

        if (columnSeparator.size() == 0)
            columnSeparator = " ";


        edit_str = escape(lineEnding);
        edit_str.copy(edit_buf_line_end, 8);

        ImGui::TextUnformatted("Line ending");
        ImGui::SameLine(secondColumnX);
        ImGui::InputText("##line-end", edit_buf_line_end, 6);

        edit_str = edit_buf_line_end;
        lineEnding = unescape(edit_str);

        if (lineEnding.size() == 0)
            lineEnding = " ";


        const char* items[] = { "dot", "comma" };
        static int item2 = 0;
        ImGui::TextUnformatted("Decimal separator");
        ImGui::SameLine(secondColumnX);
        ImGui::Combo("##dec-sep", &item2, items, 2);   // Combo using proper array. You can also pass a callback to retrieve array value, no need to create/copy an array just for that.

        decimalSeparator = item2==0 ? '.' : ',';

        ImGui::PopItemWidth();

        if(!bExportReady)
            ImGui_PushDisableButton();

        if (ImGui::Button("Copy", ImVec2(SettingsWidth/2-columnMargin, 0)) && bExportReady)
        {
            auto text=curve->get_points_text(columnSeparator, lineEnding, decimalSeparator);
            MainApp::get().copy_to_clipboard(text);
        }

        ImGui::SameLine(0.f,columnMargin*2);

        if (ImGui::Button("Export", ImVec2(SettingsWidth/2-columnMargin, 0)) && bExportReady)
            on_export_points();

        if(!bExportReady)
            ImGui_PopDisableButton();
    }


    ImGui::PushClipRect(pos,ImVec2(pos.x+SettingsWidth,(float)height),true);
    ImGui::Separator();
    ImGui::PopClipRect();

    render_hints_panel();

    ImGui::EndChild();*/
}

void MainWindow::render_hints_panel()
{
    /*if(!curve)
    {
        ImGui::TextUnformatted("Open/paste image to begin");
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

    ImGuiIO& io = ImGui::GetIO();

    if (io.KeyCtrl)
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
    ImGui::TextUnformatted(helpStr.c_str());*/
}

void MainWindow::on_export_points()
{

    int out_Result;
//    if (!curve || !curve->is_export_ready(out_Result))
//        return;

    auto path = pfd::save_file("Choose a save file", "",
            {"Text file (.txt)", "*.txt",
             "Matlab file (.mat)", "*.mat"}).result();

    if (path.empty())
        return;

    std::cout << "save: " << path << "\n";

    size_t dot_ind = path.find_last_of('.');

    bool bUseTextFormat = true;

    if (dot_ind != std::string::npos)
    {
        std::string ext = path.substr(dot_ind, path.size() - dot_ind);

        std::cout << "extension: " << ext << "\n";

        if (ext == ".txt")
        {
            std::cout << "text format\n";
        }
        else if (ext == ".mat")
        {
            bUseTextFormat = false;
            std::cout << "mat format\n";
        }
        else
        {
            auto m = pfd::message("Unknown format",
                                  "You selected unknown format\nSave file as text?",
                                  pfd::choice::yes_no_cancel,
                                  pfd::icon::question).result();

            if (m==pfd::button::no)
                on_export_points();
            if (m!=pfd::button::yes)
                return;
        }
    }
    else
    {
        auto m = pfd::message("Unknown format",
                              "You selected unknown format\nSave file as text?",
                              pfd::choice::yes_no_cancel,
                              pfd::icon::question).result();

        if (m==pfd::button::no)
            on_export_points();
        if (m!=pfd::button::yes)
            return;
    }

    std::cout << "save: " << path << "\n";

    if(bUseTextFormat)
    {
//        auto text=curve->get_points_text(columnSeparator, lineEnding, decimalSeparator);

        std::ofstream ofs;
        ofs.open(path);
//        ofs<<text;
        ofs.close();
    }
//    else
//        curve->export_points_mat_file(path.c_str());
}


void MainWindow::on_open_image()
{
    auto files = pfd::open_file("Choose an Image", "",
                                      {"Image Files (*.png, *.jpg, *.jpeg, *.bmp)",
                                       "*.png *.jpg *.jpeg *.bmp"}).result();

    if (files.empty())
        return;


    auto path = files.front();

    std::cout << "open: "<<path<<"\n";

//    auto newImage = std::make_shared<Image>(path);

//    if(!newImage->is_loaded())
//    {
//        pfd::message("Can't open image",
//                              "Opened file is not an image.\nTry again.",
//                              pfd::choice::ok,
//                              pfd::icon::error).result();
//        return;
//    }
//    else
//    {
//        image = newImage;
//        curve=std::make_shared<CurveDetect>(image);
//        currentMode = ActionMode::MODE_POINTS;
//    }

    //TODO don't reset if image was not opened
    reset_all();
}


void MainWindow::on_paste_image()
{

    std::cout << "paste from buf\n";
/*
    ImageData imageData;

    if(Clipboard::get().get_image(imageData))
    {
        image=std::make_shared<Image>(imageData);


        curve=std::make_shared<CurveDetect>(image);
        currentMode = ActionMode::MODE_POINTS;
        reset_all();
    } else
    {
        pfd::message("Can't paste image",
                     "Clipboard doesn't contain any valid image data\nTry again.",
                     pfd::choice::ok,
                     pfd::icon::error).result();
    }*/
}

void MainWindow::on_reset()
{
    auto m = pfd::message("Reset data",
                 "This will remove all input data\nAre you sure?",
                 pfd::choice::yes_no,
                 pfd::icon::question).result();
    if (m == pfd::button::yes)
    {
        reset_all();
    }
}

void MainWindow::reset_all()
{
    imageScale = 0.0f;

//    if (curve)
//    {
//        curve->reset_all();
//    }

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
    //ImGui::PopStyleColor(3);
}

void MainWindow::ImGui_PushDisableButton()
{
//    ImGui::PushStyleColor(ImGuiCol_Button, colorDisabled.Value);
//    ImGui::PushStyleColor(ImGuiCol_ButtonActive, colorDisabled.Value);
//    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorDisabled.Value);
}

