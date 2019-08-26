#ifndef CURVEDETECT_MAIN_APP_H
#define CURVEDETECT_MAIN_APP_H

#include <imgui.h>
#include "main_window.h"

#include <vector>
#include <string>
#include <imgui.h>




struct GLFWwindow;

class MainApp
{
private:
    MainApp(const MainApp&);
    MainApp& operator=(MainApp&);
    MainApp();

public:

    bool init(GLFWwindow* window, const char* glsl_version);

    bool new_frame();

    static MainApp& get();

    void copy_to_clipboard(const std::string& text);

    bool is_enter_up(){return bEnterReleased;}

private:
    const int RENDER_FRAMES_MAX=20;
    int renderFrames = RENDER_FRAMES_MAX;

    bool bEnterReleased;

    MainWindow mainWindow;

    static void on_window_resize(GLFWwindow *wnd, int width, int height);
    static void on_key_callback(GLFWwindow *wnd, int key, int scancode, int action, int mods);
    static void on_mouse_button_callback(GLFWwindow *wnd, int button, int action, int mods);
    static void on_cursor_pos_callback(GLFWwindow *wnd, double xpos, double ypos);
    static void on_scroll_callback(GLFWwindow *wnd, double xoffset, double yoffset);

    bool bShouldUseIMGUICursor;

    int m_width, m_height;

    GLFWwindow *window;

    void set_use_imgui_cursor(bool bShowCursor);


};


#endif //CURVEDETECT_MAIN_APP_H
