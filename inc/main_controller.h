#ifndef CURVEDETECT_MAIN_CONTROLLER_H
#define CURVEDETECT_MAIN_CONTROLLER_H


#include <memory>
#include <image.h>

class MainController {

public:
    void open_image();

    std::shared_ptr<Image> image;

};


#endif //CURVEDETECT_MAIN_CONTROLLER_H
