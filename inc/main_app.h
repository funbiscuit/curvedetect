#ifndef CURVEDETECT_MAIN_APP_H
#define CURVEDETECT_MAIN_APP_H

#include <imgui.h>
#include "main_window.h"

#include <vector>
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
    
    void new_frame();
    
    static MainApp& get();
    
    void copy_to_clipboard(const char* text);
    
    bool is_ctrl_down(){return bCtrlPressed;}
    bool is_shift_down(){return bShiftPressed;}
    bool is_alt_down(){return bAltPressed;}
    bool is_enter_up(){return bEnterReleased;}

private:
    
    bool bCtrlPressed;
    bool bShiftPressed;
    bool bAltPressed;
    bool bEnterReleased;
    
    MainWindow mainWindow;
    
    static void on_window_resize(GLFWwindow *wnd, int width, int height);
    static void on_key_callback(GLFWwindow *wnd, int key, int scancode, int action, int mods);
    
    bool bShouldUseIMGUICursor;
    
    int m_width, m_height;
    
    GLFWwindow *window;
    
    void set_use_imgui_cursor(bool bShowCursor);
    
    
};


#endif //CURVEDETECT_MAIN_APP_H
