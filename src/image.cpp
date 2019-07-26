
#include "image.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"


Image::Image(std::string path)
{
    texture=0;

    int c;
    image = stbi_load(path.c_str(), &width, &height, &c, 3);

    texture = ImGui_ImplOpenGL3_CreateTexture(image, width, height, false);
}

Image::~Image()
{
    if(image)
        delete(image);
    //TODO SIGSEGV if destroying after window close
//    if(texture!=0)
//        ImGui_ImplOpenGL3_DestroyTexture(texture);
}

int Image::get_width()
{
    return image ? width : 0;
}

int Image::get_height()
{
    return image ? height : 0;
}

bool Image::is_loaded()
{
    return image!=nullptr;
}

