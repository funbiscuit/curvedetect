

#ifdef __linux__

#include "resources.h"
#include "glad/glad.h"

#include <GLFW/glfw3.h>
#include <stb_image.h>

extern uint8_t _binary_icon_16_png_start;
extern uint8_t _binary_icon_32_png_start;
extern uint8_t _binary_icon_64_png_start;
extern uint8_t _binary_icon_128_png_start;
extern uint8_t _binary_icon_256_png_start;

// it is easier to get size of data with end-start expression
// using size doesn't give the correct result
//extern uint8_t _binary_icon_16_png_size;
extern uint8_t _binary_icon_16_png_end;
extern uint8_t _binary_icon_32_png_end;
extern uint8_t _binary_icon_64_png_end;
extern uint8_t _binary_icon_128_png_end;
extern uint8_t _binary_icon_256_png_end;

extern uint8_t _binary_opensans_regular_ttf_start;
extern uint8_t _binary_opensans_regular_ttf_end;


std::vector<GLFWimage> Resources::get_app_icons()
{
    uint8_t* start[]={&_binary_icon_16_png_start,
            &_binary_icon_32_png_start,
            &_binary_icon_64_png_start,
            &_binary_icon_128_png_start,
            &_binary_icon_256_png_start};
    uint32_t size[]={static_cast<uint32_t>(&_binary_icon_16_png_end-&_binary_icon_16_png_start),
                     static_cast<uint32_t>(&_binary_icon_32_png_end - &_binary_icon_16_png_start),
                     static_cast<uint32_t>(&_binary_icon_64_png_end - &_binary_icon_16_png_start),
                     static_cast<uint32_t>(&_binary_icon_128_png_end - &_binary_icon_16_png_start),
                     static_cast<uint32_t>(&_binary_icon_256_png_end - &_binary_icon_16_png_start)};

    std::vector<GLFWimage> images;
    for(int i=0;i<5;++i)
    {
        images.emplace_back();
        images.back().pixels = stbi_load_from_memory(start[i], size[i],
                                              &(images.back().width), &(images.back().height), nullptr, 4);
    }

    return images;
}

void* Resources::get_font_data(uint32_t &size)
{
    size = static_cast<uint32_t>(&_binary_opensans_regular_ttf_end-&_binary_opensans_regular_ttf_start);
    return &_binary_opensans_regular_ttf_start;
}

#endif

