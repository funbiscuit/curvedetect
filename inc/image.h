
#ifndef CURVEDETECT_IMAGE_H
#define CURVEDETECT_IMAGE_H

#include <string>

class Image {
public:
    Image(std::string path);
    ~Image();

    int get_width();
    int get_height();
    bool is_loaded();
    unsigned int get_texture() {
        return texture;
    }

private:

    unsigned char* image;

    unsigned int texture;
    int width;
    int height;

};


#endif //CURVEDETECT_IMAGE_H
