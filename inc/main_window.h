
#ifndef CURVEDETECT_MAINWINDOW_H
#define CURVEDETECT_MAINWINDOW_H

#include "main_controller.h"

class MainWindow
{
public:
    int width;
    int height;
    MainWindow();

    void on_render();
    void on_resize(int w, int h);

private:
    float toolbar_width;

    void render_toolbar();
    void render_area();
    void render_hints();

    MainController controller;
};


#endif //CURVEDETECT_MAINWINDOW_H
