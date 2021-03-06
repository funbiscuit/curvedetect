#include "main_app.h"

#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_helpers.h"
#include "glad/glad.h"
#include <resources.h>

#include <GLFW/glfw3.h>
#include "clipboard.h"


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
    glfwSetWindowIcon(window, (int) images.size(), &images[0]);



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

    float scale;
    glfwGetWindowContentScale(window, &scale, nullptr);
    if(scale<1.f)
        scale=1.f;
    uint32_t fontDataSize;
    auto fontData = Resources::get().get_font_data(fontDataSize);
    // imgui will not modify font data but it has to be non-const
    io.Fonts->AddFontFromMemoryTTF((void*)fontData, fontDataSize, scale*18.f, &config, ranges.Data);
    io.Fonts->Build();

    ImGui::GetIO().IniFilename = nullptr;

    glfwGetWindowSize(window, &m_width, &m_height);
    mainWindow.on_resize(m_width, m_height);

    glfwSetWindowSizeCallback(window, on_window_resize);
    glfwSetKeyCallback(window, on_key_callback);
    glfwSetCursorPosCallback(window, on_cursor_pos_callback);
    glfwSetMouseButtonCallback(window, on_mouse_button_callback);
    glfwSetScrollCallback(window, on_scroll_callback);

    set_use_imgui_cursor(false);

    mainWindow.init(scale);

    std::cout << "init successful\n";
    return true;
}


void MainApp::on_window_resize(GLFWwindow *wnd, int width, int height)
{
    MainApp& inst = get();
    inst.m_width = width;
    inst.m_height = height;
    inst.mainWindow.on_resize(width, height);

    inst.renderFrames = inst.RENDER_FRAMES_MAX;
    inst.new_frame();
    glfwSwapBuffers(wnd);
}


void MainApp::on_key_callback(GLFWwindow *wnd, int key, int scancode, int action, int mods)
{
    MainApp& inst = get();
    inst.renderFrames = inst.RENDER_FRAMES_MAX;

    ImGui_ImplGlfw_KeyCallback(wnd, key, scancode, action, mods);

    if (key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER)
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

void MainApp::on_cursor_pos_callback(GLFWwindow *wnd, double xpos, double ypos)
{
    MainApp& inst = get();
    inst.renderFrames = inst.RENDER_FRAMES_MAX;
}

void MainApp::on_scroll_callback(GLFWwindow *wnd, double xoffset, double yoffset)
{
    ImGui_ImplGlfw_ScrollCallback(wnd, xoffset, yoffset);
    MainApp& inst = get();
    inst.renderFrames = inst.RENDER_FRAMES_MAX;
}

void MainApp::on_mouse_button_callback(GLFWwindow *wnd, int button, int action, int mods)
{
    MainApp& inst = get();
    inst.renderFrames = inst.RENDER_FRAMES_MAX;
    ImGui_ImplGlfw_MouseButtonCallback(wnd, button, action, mods);
}

bool MainApp::new_frame()
{
    if(renderFrames<=0)
        return false;
    --renderFrames;
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

    return true;
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



void MainApp::copy_to_clipboard(const std::string& text)
{
    Clipboard::get().set_text(text);
}


