#include "main_app.h"

#include <iostream>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_helpers.h"
#include "glad/glad.h"

#include <GLFW/glfw3.h>
#include <stb_image.h>

extern stbi_uc _binary_icon_png_start;
extern stbi_uc _binary_icon_png_end;

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

    //load app icon
    GLFWimage images[1];
    stbi_uc* p=&_binary_icon_png_start;
    size_t len = &_binary_icon_png_end - &_binary_icon_png_start;
    size_t i=0;

    auto* data=new stbi_uc[len];

    while (p != &_binary_icon_png_end)
        data[i++]=*p++;

    images[0].pixels = stbi_load_from_memory(data, len, &images[0].width, &images[0].height, nullptr, 4);
    glfwSetWindowIcon(window, 1, images);
    delete[](data);

    
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
    
    ImGui::GetIO().IniFilename = nullptr;
    
    glfwGetWindowSize(window, &m_width, &m_height);
    mainWindow.on_resize(m_width, m_height);
    
    glfwSetWindowSizeCallback(window, onWindowResize);
    glfwSetKeyCallback(window, onKeyCallback);
    
    SetUseIMGUICursor(false);
    
    mainWindow.init();
    
    std::cout << "init successful\n";
    return true;
}


void MainApp::onWindowResize(GLFWwindow* wnd, int width, int height)
{
    MainApp& inst = getInstance();
    inst.m_width = width;
    inst.m_height = height;
    inst.mainWindow.on_resize(width, height);
}


void MainApp::onKeyCallback(GLFWwindow* wnd, int key, int scancode, int action, int mods)
{
    MainApp& inst = getInstance();
    
    ImGui_ImplGlfw_KeyCallback(wnd, key, scancode, action, mods);
    
    if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL)
    {
        inst.bCtrlPressed = !(action == GLFW_RELEASE);
    }
    else if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT)
    {
        inst.bShiftPressed = !(action == GLFW_RELEASE);
    }
    else if (key == GLFW_KEY_LEFT_ALT || key == GLFW_KEY_RIGHT_ALT)
    {
        inst.bAltPressed = !(action == GLFW_RELEASE);
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

MainApp& MainApp::getInstance()
{
    static MainApp instance;
    return instance;
}


void MainApp::SetUseIMGUICursor(bool bShowCursor)
{
    bool showNativeCursor = (!bShowCursor || !bShouldUseIMGUICursor);
    
    ImGui::GetIO().MouseDrawCursor = bShowCursor && bShouldUseIMGUICursor;
    
    glfwSetInputMode(window, GLFW_CURSOR, showNativeCursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
}

void MainApp::copy_to_clipboard(const char *text)
{
    glfwSetClipboardString(window, text);
}


