
#ifdef _WIN32

#include <windows.h>
#include <winuser.h>

#include "resources.h"
#include "glad/glad.h"

#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <iostream>


std::vector<GLFWimage> Resources::get_app_icons()
{
    std::vector<GLFWimage> images;
    for(int size=16;size<=256;size<<=1)
    {
        char buf[100];
        //TODO there should be a way to correctly load ICO file since it is already embedded
        sprintf(buf, "appIconPng%d", size);
        auto hResInfo = FindResourceA(nullptr, buf, RT_RCDATA);
        auto data = LoadResource(nullptr, hResInfo);
        auto len = SizeofResource(nullptr, hResInfo);

        images.emplace_back();
        images.back().pixels = stbi_load_from_memory((uint8_t*)data, len,
                &(images.back().width), &(images.back().height), nullptr, 4);
    }

    return images;
}

#endif
