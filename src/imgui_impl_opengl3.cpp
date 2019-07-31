// dear imgui: Renderer for modern OpenGL with shaders / programmatic pipeline
// - Desktop GL: 3.x 4.x
// - Embedded GL: ES 2.0 (WebGL 1.0), ES 3.0 (WebGL 2.0)
// This needs to be used along with a Platform Binding (e.g. GLFW, SDL, Win32, custom..)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'GLuint' OpenGL texture identifier as void*/ImTextureID. Read the FAQ about ImTextureID in imgui.cpp.
//  [x] Renderer: Desktop GL only: Support for large meshes (64k+ vertices) with 16-bits indices.

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you are new to dear imgui, read examples/README.txt and read the documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2019-05-29: OpenGL: Desktop GL only: Added support for large mesh (64K+ vertices), enable ImGuiBackendFlags_RendererHasVtxOffset flag.
//  2019-04-30: OpenGL: Added support for special ImDrawCallback_ResetRenderState callback to reset render state.
//  2019-03-29: OpenGL: Not calling glBindBuffer more than necessary in the render loop.
//  2019-03-15: OpenGL: Added a dummy GL call + comments in ImGui_ImplOpenGL3_Init() to detect uninitialized GL function loaders early.
//  2019-03-03: OpenGL: Fix support for ES 2.0 (WebGL 1.0).
//  2019-02-20: OpenGL: Fix for OSX not supporting OpenGL 4.5, we don't try to read GL_CLIP_ORIGIN even if defined by the headers/loader.
//  2019-02-11: OpenGL: Projecting clipping rectangles correctly using draw_data->FramebufferScale to allow multi-viewports for retina display.
//  2019-02-01: OpenGL: Using GLSL 410 shaders for any version over 410 (e.g. 430, 450).
//  2018-11-30: Misc: Setting up io.BackendRendererName so it can be displayed in the About Window.
//  2018-11-13: OpenGL: Support for GL 4.5's glClipControl(GL_UPPER_LEFT) / GL_CLIP_ORIGIN.
//  2018-08-29: OpenGL: Added support for more OpenGL loaders: glew and glad, with comments indicative that any loader can be used.
//  2018-08-09: OpenGL: Default to OpenGL ES 3 on iOS and Android. GLSL version default to "#version 300 ES".
//  2018-07-30: OpenGL: Support for GLSL 300 ES and 410 core. Fixes for Emscripten compilation.
//  2018-07-10: OpenGL: Support for more GLSL versions (based on the GLSL version string). Added error output when shaders fail to compile/link.
//  2018-06-08: Misc: Extracted imgui_impl_opengl3.cpp/.h away from the old combined GLFW/SDL+OpenGL3 examples.
//  2018-06-08: OpenGL: Use draw_data->DisplayPos and draw_data->DisplaySize to setup projection matrix and clipping rectangle.
//  2018-05-25: OpenGL: Removed unnecessary backup/restore of GL_ELEMENT_ARRAY_BUFFER_BINDING since this is part of the VAO state.
//  2018-05-14: OpenGL: Making the call to glBindSampler() optional so 3.2 context won't fail if the function is a NULL pointer.
//  2018-03-06: OpenGL: Added const char* glsl_version parameter to ImGui_ImplOpenGL3_Init() so user can override the GLSL version e.g. "#version 150".
//  2018-02-23: OpenGL: Create the VAO in the render function so the setup can more easily be used with multiple shared GL context.
//  2018-02-16: Misc: Obsoleted the io.RenderDrawListsFn callback and exposed ImGui_ImplSdlGL3_RenderDrawData() in the .h file so you can call it yourself.
//  2018-01-07: OpenGL: Changed GLSL shader version from 330 to 150.
//  2017-09-01: OpenGL: Save and restore current bound sampler. Save and restore current polygon mode.
//  2017-05-01: OpenGL: Fixed save and restore of current blend func state.
//  2017-05-01: OpenGL: Fixed save and restore of current GL_ACTIVE_TEXTURE.
//  2016-09-05: OpenGL: Fixed save and restore of current scissor rectangle.
//  2016-07-29: OpenGL: Explicitly setting GL_UNPACK_ROW_LENGTH to reduce issues because SDL changes it. (#752)

//----------------------------------------
// OpenGL    GLSL      GLSL
// version   version   string
//----------------------------------------
//  2.0       110       "#version 110"
//  2.1       120       "#version 120"
//  3.0       130       "#version 130"
//  3.1       140       "#version 140"
//  3.2       150       "#version 150"
//  3.3       330       "#version 330 core"
//  4.0       400       "#version 400 core"
//  4.1       410       "#version 410 core"
//  4.2       420       "#version 410 core"
//  4.3       430       "#version 430 core"
//  ES 2.0    100       "#version 100"      = WebGL 1.0
//  ES 3.0    300       "#version 300 es"   = WebGL 2.0
//----------------------------------------

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "glad/glad.h"
#include <stdio.h>
#if defined(_MSC_VER) && _MSC_VER <= 1500 // MSVC 2008 or earlier
#include <stddef.h>     // intptr_t
#else
#include <stdint.h>     // intptr_t
#endif
#if defined(__APPLE__)
#include "TargetConditionals.h"
#endif

// Auto-detect GL version
#if !defined(IMGUI_IMPL_OPENGL_ES2) && !defined(IMGUI_IMPL_OPENGL_ES3)
#if (defined(__APPLE__) && (TARGET_OS_IOS || TARGET_OS_TV)) || (defined(__ANDROID__))
#define IMGUI_IMPL_OPENGL_ES3       // iOS, Android  -> GL ES 3, "#version 300 es"
#elif defined(__EMSCRIPTEN__)
#define IMGUI_IMPL_OPENGL_ES2       // Emscripten    -> GL ES 2, "#version 100"
#endif
#endif


// Desktop GL has glDrawElementsBaseVertex() which GL ES and WebGL don't have.
#if defined(IMGUI_IMPL_OPENGL_ES2) || defined(IMGUI_IMPL_OPENGL_ES3)
#define IMGUI_IMPL_OPENGL_HAS_DRAW_WITH_BASE_VERTEX     0
#else
#define IMGUI_IMPL_OPENGL_HAS_DRAW_WITH_BASE_VERTEX     1
#endif


// Data
static int			g_ImageTexID = -1;
static bool			g_ImageMakeBin = false;
static int			g_BinarizationLevel = 127;

// OpenGL Data
static char         g_GlslVersionString[32] = "";
static GLuint       g_FontTexture = 0;
static GLuint       g_ShaderHandle = 0, g_VertHandle = 0, g_FragHandle = 0;

static int			g_AttribLocationMakeBin = 0;
static int			g_AttribLocationMakeGray = 0;
static int			g_AttribLocationBinLevel = 0;

static int          g_AttribLocationTex = 0, g_AttribLocationProjMtx = 0;                                // Uniforms location
static int          g_AttribLocationVtxPos = 0, g_AttribLocationVtxUV = 0, g_AttribLocationVtxColor = 0; // Vertex attributes location
static unsigned int g_VboHandle = 0, g_ElementsHandle = 0;


void ImGui_ImplOpenGL3_SetImageBin(int ID, bool makeBin)
{
    g_ImageTexID = ID;
    g_ImageMakeBin = makeBin;
}

void ImGui_ImplOpenGL3_SetBinarizationLevel(int Level)
{
    g_BinarizationLevel = Level;
}


// Functions
bool    ImGui_ImplOpenGL3_Init(const char* glsl_version)
{
    // Setup back-end capabilities flags
    ImGuiIO& io = ImGui::GetIO();
    io.BackendRendererName = "imgui_impl_opengl3";
#if IMGUI_IMPL_OPENGL_HAS_DRAW_WITH_BASE_VERTEX
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
#endif

    // Store GLSL version string so we can refer to it later in case we recreate shaders. Note: GLSL version is NOT the same as GL version. Leave this to NULL if unsure.
#if defined(IMGUI_IMPL_OPENGL_ES2)
    if (glsl_version == NULL)
        glsl_version = "#version 100";
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    if (glsl_version == NULL)
        glsl_version = "#version 300 es";
#else
    if (glsl_version == NULL)
        glsl_version = "#version 130";
#endif
    IM_ASSERT((int)strlen(glsl_version) + 2 < IM_ARRAYSIZE(g_GlslVersionString));
    strcpy(g_GlslVersionString, glsl_version);
    strcat(g_GlslVersionString, "\n");

    // Make a dummy GL call (we don't actually need the result)
    // IF YOU GET A CRASH HERE: it probably means that you haven't initialized the OpenGL function loader used by this code.
    // Desktop OpenGL 3/4 need a function loader. See the IMGUI_IMPL_OPENGL_LOADER_xxx explanation above.
    GLint current_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &current_texture);

    return true;
}

void    ImGui_ImplOpenGL3_Shutdown()
{
    ImGui_ImplOpenGL3_DestroyDeviceObjects();
}

void    ImGui_ImplOpenGL3_NewFrame()
{
    if (!g_FontTexture)
        ImGui_ImplOpenGL3_CreateDeviceObjects();
}

static void ImGui_ImplOpenGL3_SetupRenderState(ImDrawData* draw_data, int fb_width, int fb_height, GLuint vertex_array_object)
{
    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
    glad_glEnable(GL_BLEND);
    glad_glBlendEquation(GL_FUNC_ADD);
    glad_glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glad_glDisable(GL_CULL_FACE);
    glad_glDisable(GL_DEPTH_TEST);
    glad_glEnable(GL_SCISSOR_TEST);
#ifdef GL_POLYGON_MODE
    glad_glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

    // Setup viewport, orthographic projection matrix
    // Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for singlad_gle viewport apps.
    glad_glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
    float L = draw_data->DisplayPos.x;
    float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
    float T = draw_data->DisplayPos.y;
    float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
    const float ortho_projection[4][4] =
    {
        { 2.0f/(R-L),   0.0f,         0.0f,   0.0f },
        { 0.0f,         2.0f/(T-B),   0.0f,   0.0f },
        { 0.0f,         0.0f,        -1.0f,   0.0f },
        { (R+L)/(L-R),  (T+B)/(B-T),  0.0f,   1.0f },
    };
    glad_glUseProgram(g_ShaderHandle);
    glad_glUniform1i(g_AttribLocationTex, 0);
    glad_glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
#ifdef GL_SAMPLER_BINDING
    glad_glBindSampler(0, 0); // We use combined texture/sampler state. Applications using GL 3.3 may set that otherwise.
#endif

    (void)vertex_array_object;
#ifndef IMGUI_IMPL_OPENGL_ES2
    glad_glBindVertexArray(vertex_array_object);
#endif

    // Bind vertex/index buffers and setup attributes for ImDrawVert
    glad_glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
    glad_glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ElementsHandle);
    glad_glEnableVertexAttribArray(g_AttribLocationVtxPos);
    glad_glEnableVertexAttribArray(g_AttribLocationVtxUV);
    glad_glEnableVertexAttribArray(g_AttribLocationVtxColor);
    glad_glVertexAttribPointer(g_AttribLocationVtxPos,   2, GL_FLOAT,         GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
    glad_glVertexAttribPointer(g_AttribLocationVtxUV,    2, GL_FLOAT,         GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
    glad_glVertexAttribPointer(g_AttribLocationVtxColor, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, col));
}

// OpenGL3 Render function.
// (this used to be set in io.RenderDrawListsFn and called by ImGui::Render(), but you can now call this directly from your main loop)
// Note that this implementation is little overcomplicated because we are saving/setting up/restoring every OpenGL state explicitly, in order to be able to run within any OpenGL engine that doesn't do so.
void    ImGui_ImplOpenGL3_RenderDrawData(ImDrawData* draw_data)
{
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0)
        return;

    // Backup GL state
    GLenum last_active_texture; glad_glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
    glad_glActiveTexture(GL_TEXTURE0);
    GLint last_program; glad_glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
    GLint last_texture; glad_glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
#ifdef GL_SAMPLER_BINDING
    GLint last_sampler; glad_glGetIntegerv(GL_SAMPLER_BINDING, &last_sampler);
#endif
    GLint last_array_buffer; glad_glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
#ifndef IMGUI_IMPL_OPENGL_ES2
    GLint last_vertex_array_object; glad_glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array_object);
#endif
#ifdef GL_POLYGON_MODE
    GLint last_polygon_mode[2]; glad_glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
#endif
    GLint last_viewport[4]; glad_glGetIntegerv(GL_VIEWPORT, last_viewport);
    GLint last_scissor_box[4]; glad_glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
    GLenum last_blend_src_rgb; glad_glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
    GLenum last_blend_dst_rgb; glad_glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
    GLenum last_blend_src_alpha; glad_glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
    GLenum last_blend_dst_alpha; glad_glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
    GLenum last_blend_equation_rgb; glad_glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
    GLenum last_blend_equation_alpha; glad_glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
    GLboolean last_enable_blend = glad_glIsEnabled(GL_BLEND);
    GLboolean last_enable_cull_face = glad_glIsEnabled(GL_CULL_FACE);
    GLboolean last_enable_depth_test = glad_glIsEnabled(GL_DEPTH_TEST);
    GLboolean last_enable_scissor_test = glad_glIsEnabled(GL_SCISSOR_TEST);
    bool clip_origin_lower_left = true;
#if defined(GL_CLIP_ORIGIN) && !defined(__APPLE__)
    GLenum last_clip_origin = 0; glad_glGetIntegerv(GL_CLIP_ORIGIN, (GLint*)&last_clip_origin); // Support for GL 4.5's glad_glClipControl(GL_UPPER_LEFT)
    if (last_clip_origin == GL_UPPER_LEFT)
        clip_origin_lower_left = false;
#endif

    // Setup desired GL state
    // Recreate the VAO every time (this is to easily allow multiple GL contexts to be rendered to. VAO are not shared among GL contexts)
    // The renderer would actually work without any VAO bound, but then our VertexAttrib calls would overwrite the default one currently bound.
    GLuint vertex_array_object = 0;
#ifndef IMGUI_IMPL_OPENGL_ES2
    glad_glGenVertexArrays(1, &vertex_array_object);
#endif
    ImGui_ImplOpenGL3_SetupRenderState(draw_data, fb_width, fb_height, vertex_array_object);

    // Will project scissor/clipping rectangles into framebuffer space
    ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
    ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

    // Render command lists
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];

        // Upload vertex/index buffers
        glad_glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);
        glad_glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != NULL)
            {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                    ImGui_ImplOpenGL3_SetupRenderState(draw_data, fb_width, fb_height, vertex_array_object);
                else
                    pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                // Project scissor/clipping rectangles into framebuffer space
                ImVec4 clip_rect;
                clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
                clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
                clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
                clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

                if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
                {
                    // Apply scissor/clipping rectangle
                    if (clip_origin_lower_left)
                        glad_glScissor((int)clip_rect.x, (int)(fb_height - clip_rect.w), (int)(clip_rect.z - clip_rect.x), (int)(clip_rect.w - clip_rect.y));
                    else
                        glad_glScissor((int)clip_rect.x, (int)clip_rect.y, (int)clip_rect.z, (int)clip_rect.w); // Support for GL 4.5 rarely used glad_glClipControl(GL_UPPER_LEFT)
    
                    if (g_ImageTexID == (intptr_t)pcmd->TextureId)
                    {
                        glUniform1i(g_AttribLocationMakeGray, 1);
                        glUniform1i(g_AttribLocationMakeBin, g_ImageMakeBin);
                        glUniform1f(g_AttribLocationBinLevel, float(g_BinarizationLevel) / 255.0f);
                    }
                    else
                    {
                        glUniform1i(g_AttribLocationMakeGray, 0);
                        glUniform1i(g_AttribLocationMakeBin, 0);
                    }
                    
                    // Bind texture, Draw
                    glad_glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
#if IMGUI_IMPL_OPENGL_HAS_DRAW_WITH_BASE_VERTEX
                    glad_glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void*)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx)), (GLint)pcmd->VtxOffset);
#else
                    glad_glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void*)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx)));
#endif
                }
            }
        }
    }

    // Destroy the temporary VAO
#ifndef IMGUI_IMPL_OPENGL_ES2
    glad_glDeleteVertexArrays(1, &vertex_array_object);
#endif

    // Restore modified GL state
    glad_glUseProgram(last_program);
    glad_glBindTexture(GL_TEXTURE_2D, last_texture);
#ifdef GL_SAMPLER_BINDING
    glad_glBindSampler(0, last_sampler);
#endif
    glad_glActiveTexture(last_active_texture);
#ifndef IMGUI_IMPL_OPENGL_ES2
    glad_glBindVertexArray(last_vertex_array_object);
#endif
    glad_glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
    glad_glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
    glad_glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
    if (last_enable_blend) glad_glEnable(GL_BLEND); else glad_glDisable(GL_BLEND);
    if (last_enable_cull_face) glad_glEnable(GL_CULL_FACE); else glad_glDisable(GL_CULL_FACE);
    if (last_enable_depth_test) glad_glEnable(GL_DEPTH_TEST); else glad_glDisable(GL_DEPTH_TEST);
    if (last_enable_scissor_test) glad_glEnable(GL_SCISSOR_TEST); else glad_glDisable(GL_SCISSOR_TEST);
#ifdef GL_POLYGON_MODE
    glad_glPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]);
#endif
    glad_glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
    glad_glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
}

bool ImGui_ImplOpenGL3_CreateFontsTexture()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

    g_FontTexture = ImGui_ImplOpenGL3_CreateTexture(pixels, width, height, true);

    // Store our identifier
    io.Fonts->TexID = (ImTextureID)(intptr_t)g_FontTexture;

    return true;
}

void ImGui_ImplOpenGL3_DestroyFontsTexture()
{
    if (g_FontTexture)
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui_ImplOpenGL3_DestroyTexture(g_FontTexture);
        io.Fonts->TexID = 0;
        g_FontTexture = 0;
    }
}

unsigned int ImGui_ImplOpenGL3_CreateTexture(unsigned char* pixels, int width, int height, bool withAlpha, bool smooth)
{
    GLuint texture=0;
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    // Upload texture to graphics system
    GLint last_texture;
    glad_glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glad_glGenTextures(1, &texture);
    glad_glBindTexture(GL_TEXTURE_2D, texture);
    
    glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, smooth ? GL_LINEAR : GL_NEAREST);
    glad_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, smooth ? GL_LINEAR : GL_NEAREST);
#ifdef GL_UNPACK_ROW_LENGTH
    glad_glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif

    glad_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, withAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, pixels);

    // Restore state
    glad_glBindTexture(GL_TEXTURE_2D, (GLuint)last_texture);

    return texture;
}

void ImGui_ImplOpenGL3_DestroyTexture(unsigned int id)
{
    if(glad_glIsTexture(id))
        glad_glDeleteTextures(1, &id);
}

// If you get an error please report on github. You may try different GL context version or GLSL version. See GL<>GLSL version table at the top of this file.
static bool CheckShader(GLuint handle, const char* desc)
{
    GLint status = 0, log_length = 0;
    glad_glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
    glad_glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_length);
    if ((GLboolean)status == GL_FALSE)
        fprintf(stderr, "ERROR: ImGui_ImplOpenGL3_CreateDeviceObjects: failed to compile %s!\n", desc);
    if (log_length > 1)
    {
        ImVector<char> buf;
        buf.resize((int)(log_length + 1));
        glad_glGetShaderInfoLog(handle, log_length, NULL, (GLchar*)buf.begin());
        fprintf(stderr, "%s\n", buf.begin());
    }
    return (GLboolean)status == GL_TRUE;
}

// If you get an error please report on GitHub. You may try different GL context version or GLSL version.
static bool CheckProgram(GLuint handle, const char* desc)
{
    GLint status = 0, log_length = 0;
    glad_glGetProgramiv(handle, GL_LINK_STATUS, &status);
    glad_glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_length);
    if ((GLboolean)status == GL_FALSE)
        fprintf(stderr, "ERROR: ImGui_ImplOpenGL3_CreateDeviceObjects: failed to link %s! (with GLSL '%s')\n", desc, g_GlslVersionString);
    if (log_length > 1)
    {
        ImVector<char> buf;
        buf.resize((int)(log_length + 1));
        glad_glGetProgramInfoLog(handle, log_length, NULL, (GLchar*)buf.begin());
        fprintf(stderr, "%s\n", buf.begin());
    }
    return (GLboolean)status == GL_TRUE;
}

bool    ImGui_ImplOpenGL3_CreateDeviceObjects()
{
    // Backup GL state
    GLint last_texture, last_array_buffer;
    glad_glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glad_glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
#ifndef IMGUI_IMPL_OPENGL_ES2
    GLint last_vertex_array;
    glad_glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
#endif

    int glsl_version = 130;

    const GLchar* vertex_shader =
        "uniform mat4 ProjMtx;\n"
        "in vec2 Position;\n"
        "in vec2 UV;\n"
        "in vec4 Color;\n"
        "out vec2 Frag_UV;\n"
        "out vec4 Frag_Color;\n"
        "void main()\n"
        "{\n"
        "    Frag_UV = UV;\n"
        "    Frag_Color = Color;\n"
        "    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
        "}\n";

    //simple shader that accounts for binarization of image
    const GLchar* fragment_shader =
        "uniform sampler2D Texture;\n"
        "uniform bool bMakeBin;\n"
        "uniform bool bMakeGray;\n"
        "uniform float ColorLevel;\n"
        "in vec2 Frag_UV;\n"
        "in vec4 Frag_Color;\n"
        "out vec4 Out_Color;\n"
        "void main()\n"
        "{\n"
        "	vec4 TexColor = texture( Texture, Frag_UV.st);\n"
        "	float gray = 0.216f*TexColor.x+0.7152f*TexColor.y+0.0722*TexColor.z;\n"
        "	if(bMakeBin)\n"
        "	{\n"
        "		if(gray < ColorLevel)\n"
        "		{\n"
        "			TexColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);\n"
        "		}\n"
        "		else\n"
        "		{\n"
        "			TexColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
        "		}\n"
        "	}\n"
        "	else if (bMakeGray)\n"
        "	    TexColor = vec4(gray, gray, gray, 1.0f);\n"
        "	Out_Color = Frag_Color * TexColor;\n"
        "}\n";


    // Create shaders
    const GLchar* vertex_shader_with_version[2] = { g_GlslVersionString, vertex_shader };
    g_VertHandle = glad_glCreateShader(GL_VERTEX_SHADER);
    glad_glShaderSource(g_VertHandle, 2, vertex_shader_with_version, NULL);
    glad_glCompileShader(g_VertHandle);
    CheckShader(g_VertHandle, "vertex shader");

    const GLchar* fragment_shader_with_version[2] = { g_GlslVersionString, fragment_shader };
    g_FragHandle = glad_glCreateShader(GL_FRAGMENT_SHADER);
    glad_glShaderSource(g_FragHandle, 2, fragment_shader_with_version, NULL);
    glad_glCompileShader(g_FragHandle);
    CheckShader(g_FragHandle, "fragment shader");

    g_ShaderHandle = glad_glCreateProgram();
    glad_glAttachShader(g_ShaderHandle, g_VertHandle);
    glad_glAttachShader(g_ShaderHandle, g_FragHandle);
    glad_glLinkProgram(g_ShaderHandle);
    CheckProgram(g_ShaderHandle, "shader program");
    
    g_AttribLocationMakeBin = glad_glGetUniformLocation(g_ShaderHandle, "bMakeBin");
    g_AttribLocationMakeGray = glad_glGetUniformLocation(g_ShaderHandle, "bMakeGray");
    g_AttribLocationBinLevel = glad_glGetUniformLocation(g_ShaderHandle, "ColorLevel");
    
    g_AttribLocationTex = glad_glGetUniformLocation(g_ShaderHandle, "Texture");
    g_AttribLocationProjMtx = glad_glGetUniformLocation(g_ShaderHandle, "ProjMtx");
    g_AttribLocationVtxPos = glad_glGetAttribLocation(g_ShaderHandle, "Position");
    g_AttribLocationVtxUV = glad_glGetAttribLocation(g_ShaderHandle, "UV");
    g_AttribLocationVtxColor = glad_glGetAttribLocation(g_ShaderHandle, "Color");

    // Create buffers
    glad_glGenBuffers(1, &g_VboHandle);
    glad_glGenBuffers(1, &g_ElementsHandle);

    ImGui_ImplOpenGL3_CreateFontsTexture();

    // Restore modified GL state
    glad_glBindTexture(GL_TEXTURE_2D, last_texture);
    glad_glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
#ifndef IMGUI_IMPL_OPENGL_ES2
    glad_glBindVertexArray(last_vertex_array);
#endif

    return true;
}

void    ImGui_ImplOpenGL3_DestroyDeviceObjects()
{
    if (g_VboHandle) glad_glDeleteBuffers(1, &g_VboHandle);
    if (g_ElementsHandle) glad_glDeleteBuffers(1, &g_ElementsHandle);
    g_VboHandle = g_ElementsHandle = 0;

    if (g_ShaderHandle && g_VertHandle) glad_glDetachShader(g_ShaderHandle, g_VertHandle);
    if (g_VertHandle) glad_glDeleteShader(g_VertHandle);
    g_VertHandle = 0;

    if (g_ShaderHandle && g_FragHandle) glad_glDetachShader(g_ShaderHandle, g_FragHandle);
    if (g_FragHandle) glad_glDeleteShader(g_FragHandle);
    g_FragHandle = 0;

    if (g_ShaderHandle) glad_glDeleteProgram(g_ShaderHandle);
    g_ShaderHandle = 0;

    ImGui_ImplOpenGL3_DestroyFontsTexture();
}
