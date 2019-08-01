
#ifndef CURVEDETECT_RESOURCES_H
#define CURVEDETECT_RESOURCES_H

#include <cstdint>
#include <vector>

struct GLFWimage;

class Resources
{
private:
    Resources(const Resources&);
    Resources& operator=(Resources&);
    Resources() = default;

public:

    static Resources& get();

    std::vector<GLFWimage> get_app_icons();

    /**
     * Returns font data as byte array. Array size will be stored in size param
     * @param size
     * @return
     */
    void* get_font_data(uint32_t& size);
};

#endif //CURVEDETECT_RESOURCES_H
