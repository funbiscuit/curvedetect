#include <iostream>
#include <chrono>
#include <thread>
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <main_window.h>
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

    MainWindow mainWindow;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(mainWindow.width, mainWindow.height, "CurveDetect", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync


    // setup glad

    if(!gladLoadGL()) {
        printf("Cant setup glad! Something went wrong!\n");
        return -1;
    }
    printf("OpenGL %d.%d\n", GLVersion.major, GLVersion.minor);



    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsLight();
//    ImGui::StyleColorsDark();
//    ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'misc/fonts/README.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    ImVec4 clear_color = ImVec4(0.9f, 0.9f, 0.9f, 1.00f);


    glfwSetWindowSizeLimits(window, 800, 600, GLFW_DONT_CARE, GLFW_DONT_CARE);

    auto start=std::chrono::high_resolution_clock::now();

    int w,h;
    int prevW=mainWindow.width, prevH=mainWindow.height;

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glfwGetWindowSize(window, &w, &h);
        if(w!=prevW || h!=prevH)
        {
            prevW=w;prevH=h;
            mainWindow.on_resize(w,h);
        }

        mainWindow.on_render();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        glad_glViewport(0, 0, display_w, display_h);
        glad_glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glad_glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        auto end=std::chrono::high_resolution_clock::now();
        auto mseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        while(mseconds<minMillis)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            end=std::chrono::high_resolution_clock::now();
            mseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
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

