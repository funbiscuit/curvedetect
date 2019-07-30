

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

extern uint64_t _binary_icon_16_png_size;
extern uint64_t _binary_icon_32_png_size;
extern uint64_t _binary_icon_64_png_size;
extern uint64_t _binary_icon_128_png_size;
extern uint64_t _binary_icon_256_png_size;



std::vector<GLFWimage> Resources::get_app_icons()
{
    uint8_t* start[]={&_binary_icon_16_png_start,
            &_binary_icon_32_png_start,
            &_binary_icon_64_png_start,
            &_binary_icon_128_png_start,
            &_binary_icon_256_png_start};
    uint64_t size[]={_binary_icon_16_png_size,
                     _binary_icon_32_png_size,
                     _binary_icon_64_png_size,
                     _binary_icon_128_png_size,
                     _binary_icon_256_png_size};

    std::vector<GLFWimage> images;
    for(int i=0;i<5;++i)
    {
        images.emplace_back();
        images.back().pixels = stbi_load_from_memory(start[i], size[i],
                                              &(images.back().width), &(images.back().height), nullptr, 4);
    }

    return images;
}

#endif

