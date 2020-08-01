
#include "resource_builder/resources.h"
#include "resources.h"

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <stb_image.h>

Resources& Resources::get()
{
    static Resources instance;
    return instance;
}

std::vector<GLFWimage> Resources::get_app_icons()
{
    ResourceBuilder::ResourceId icons[] = {ResourceBuilder::RES___ICONS_ICON_16_PNG,
                                           ResourceBuilder::RES___ICONS_ICON_32_PNG,
                                           ResourceBuilder::RES___ICONS_ICON_64_PNG,
                                           ResourceBuilder::RES___ICONS_ICON_128_PNG,
                                           ResourceBuilder::RES___ICONS_ICON_256_PNG,
    };

    std::vector<GLFWimage> images;
    for(int i=0;i<5;++i)
    {
        images.emplace_back();
        auto data = ResourceBuilder::get_resource_data(icons[i]);
        auto size = ResourceBuilder::get_resource_size(icons[i]);

        images.back().pixels = stbi_load_from_memory(data, size,
                                                     &(images.back().width), &(images.back().height), nullptr, 4);
    }

    return images;
}

const uint8_t* Resources::get_font_data(uint32_t &size)
{
    size = ResourceBuilder::get_resource_size(ResourceBuilder::RES___OPENSANS_REGULAR_TTF);
    return ResourceBuilder::get_resource_data(ResourceBuilder::RES___OPENSANS_REGULAR_TTF);
}
