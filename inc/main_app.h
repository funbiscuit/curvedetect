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
    
    static MainApp& getInstance();
    
    void copy_to_clipboard(const char* text);
    
    bool isCtrlPressed(){return bCtrlPressed;}
    bool isShiftPressed(){return bShiftPressed;}
    bool isAltPressed(){return bAltPressed;}
    bool isEnterReleased(){return bEnterReleased;}

private:
    
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
    
    
};


#endif //CURVEDETECT_MAIN_APP_H
