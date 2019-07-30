
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

    static Resources& getInstance();

    std::vector<GLFWimage> get_app_icons();

};

#endif //CURVEDETECT_RESOURCES_H
