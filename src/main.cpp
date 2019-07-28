#include <iostream>
#include <chrono>
#include <thread>
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <main_window.h>
#include <main_app.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

int init()
{
    int minMillis=10;


    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;


    // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif


    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1280, 720, "CurveDetect", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    
    glfwSetWindowSizeLimits(window, 800, 500, GLFW_DONT_CARE, GLFW_DONT_CARE);
    
    MainApp& app = MainApp::getInstance();
    if(!app.init(window, glsl_version))
    {
        std::cout << "Can't initialize application\n";
        return -1;
    }
    
    auto start=std::chrono::high_resolution_clock::now();
    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();
    
        app.new_frame();

        glfwSwapBuffers(window);
        auto end=std::chrono::high_resolution_clock::now();
        auto mseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

//        while(mseconds<minMillis)
//        {
//            std::this_thread::sleep_for(std::chrono::milliseconds(1));
//            end=std::chrono::high_resolution_clock::now();
//            mseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
//        }
    
        //if we updated very fast - sleep a little bit
        if (mseconds < minMillis)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(minMillis-mseconds));
        }

        start=std::chrono::high_resolution_clock::now();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();


    return 0;
}


int main() {
    return init();
}

