

#ifdef _WIN32

#include "clipboard.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>

void Clipboard::init_platform()
{
    //nothing to do
}

void Clipboard::cleanup_platform()
{
    //nothing to do
}


void Clipboard::set_text(std::string text)
{
    //on windows this works okay
    glfwSetClipboardString(nullptr, text.c_str());
}

bool Clipboard::get_image(ImageData& imageData)
{
    //TODO not supported yet

    return false;
}

#endif
