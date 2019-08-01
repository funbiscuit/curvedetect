#include "main_app.h"

#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_helpers.h"
#include "glad/glad.h"
#include <resources.h>

#include <GLFW/glfw3.h>


MainApp::MainApp()
{
    window = nullptr;
    
    m_width = 600;
    m_height = 400;
    
    
    bShouldUseIMGUICursor = false;
}


bool MainApp::init(GLFWwindow *wnd, const char* glsl_version)
{
    // setup glad
    if(!gladLoadGL()) {
        printf("Cant setup glad! Something went wrong!\n");
        return false;
    }
    printf("OpenGL %d.%d\n", GLVersion.major, GLVersion.minor);
    
    glad_glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    window = wnd;

    //load app icons
    auto images= Resources::get().get_app_icons();
    glfwSetWindowIcon(window, images.size(), &images[0]);



    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsLight();
//    ImGui::StyleColorsDark();
//    ImGui::StyleColorsClassic();
    
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    
    ImFontConfig config;
    config.OversampleH = 3;
    config.OversampleV = 2;
    config.FontDataOwnedByAtlas = false;    //font data should not be deleted by imgui
    config.RasterizerMultiply = 0.85f;      //this will brighten font a little bit making it nicer

    ImVector<ImWchar> ranges;
    ImFontGlyphRangesBuilder builder;
    builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
    builder.BuildRanges(&ranges);

    uint32_t fontDataSize;
    auto fontData = Resources::get().get_font_data(fontDataSize);
    io.Fonts->AddFontFromMemoryTTF(fontData, fontDataSize, 18.f, &config, ranges.Data);
    io.Fonts->Build();

    ImGui::GetIO().IniFilename = nullptr;
    
    glfwGetWindowSize(window, &m_width, &m_height);
    mainWindow.on_resize(m_width, m_height);

    glfwSetWindowSizeCallback(window, on_window_resize);
    glfwSetKeyCallback(window, on_key_callback);

    set_use_imgui_cursor(false);
    
    mainWindow.init();
    
    std::cout << "init successful\n";
    return true;
}


void MainApp::on_window_resize(GLFWwindow *wnd, int width, int height)
{
    MainApp& inst = get();
    inst.m_width = width;
    inst.m_height = height;
    inst.mainWindow.on_resize(width, height);
}


void MainApp::on_key_callback(GLFWwindow *wnd, int key, int scancode, int action, int mods)
{
    MainApp& inst = get();
    
    ImGui_ImplGlfw_KeyCallback(wnd, key, scancode, action, mods);
    
    if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL)
    {
        inst.bCtrlPressed = action != GLFW_RELEASE;
    }
    else if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT)
    {
        inst.bShiftPressed = action != GLFW_RELEASE;
    }
    else if (key == GLFW_KEY_LEFT_ALT || key == GLFW_KEY_RIGHT_ALT)
    {
        inst.bAltPressed = action != GLFW_RELEASE;
    }
    else if (key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER)
    {
        inst.bEnterReleased = (action == GLFW_RELEASE);
    }
    else if (key == GLFW_KEY_UP)
    {
        if (action == GLFW_PRESS || action == GLFW_REPEAT)
        {
            double xpos, ypos;
            glfwGetCursorPos(wnd, &xpos, &ypos);
            ypos -= 1;
            glfwSetCursorPos(wnd, xpos, ypos);
        }
    }
    else if (key == GLFW_KEY_DOWN)
    {
        if (action == GLFW_PRESS || action == GLFW_REPEAT)
        {
            double xpos, ypos;
            glfwGetCursorPos(wnd, &xpos, &ypos);
            ypos += 1;
            glfwSetCursorPos(wnd, xpos, ypos);
        }
    }
    else if (key == GLFW_KEY_LEFT)
    {
        if (action == GLFW_PRESS || action == GLFW_REPEAT)
        {
            double xpos, ypos;
            glfwGetCursorPos(wnd, &xpos, &ypos);
            xpos -= 1;
            glfwSetCursorPos(wnd, xpos, ypos);
        }
    }
    else if (key == GLFW_KEY_RIGHT)
    {
        if (action == GLFW_PRESS || action == GLFW_REPEAT)
        {
            double xpos, ypos;
            glfwGetCursorPos(wnd, &xpos, &ypos);
            xpos += 1;
            glfwSetCursorPos(wnd, xpos, ypos);
        }
    }
}

void MainApp::new_frame()
{
    ImVec4 clear_color = ImVec4(0.9f, 0.9f, 0.9f, 1.00f);
    
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    mainWindow.on_render();
    
    bEnterReleased = false;
    
    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    
    glad_glViewport(0, 0, display_w, display_h);
    glad_glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glad_glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

MainApp& MainApp::get()
{
    static MainApp instance;
    return instance;
}


void MainApp::set_use_imgui_cursor(bool bShowCursor)
{
    bool showNativeCursor = (!bShowCursor || !bShouldUseIMGUICursor);
    
    ImGui::GetIO().MouseDrawCursor = bShowCursor && bShouldUseIMGUICursor;
    
    glfwSetInputMode(window, GLFW_CURSOR, showNativeCursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
}

void MainApp::copy_to_clipboard(const char *text)
{
    glfwSetClipboardString(window, text);
}


