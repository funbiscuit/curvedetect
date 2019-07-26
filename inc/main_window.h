
#ifndef CURVEDETECT_MAINWINDOW_H
#define CURVEDETECT_MAINWINDOW_H

class MainWindow
{
public:
    int width;
    int height;
    MainWindow();

    void on_render();

private:
    float toolbar_width;

    void render_toolbar();
    void render_area();
    void render_hints();

};


#endif //CURVEDETECT_MAINWINDOW_H
