
#ifndef CURVEDETECT_IMAGE_H
#define CURVEDETECT_IMAGE_H

#include <string>
#include <Eigen/Dense>
using Eigen::MatrixXi;

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
    
    bool isPixelInside(int px, int py);
    bool getClosestBlack(int& px, int& py, int hside, int ColorLevel);
    
    
    int getPixelValue(int px, int py);
    
    //px and py will be replaced with the local coordinates
    //inside of returned region
    //region will be a square with side = (2*hside+1)
    bool getNearbyPoints(int& px, int& py, int hside, MatrixXi& out_PointRegion);

private:

    unsigned char* image;

    unsigned int texture;
    int width;
    int height;
    
    MatrixXi ImageMatrix;

};


#endif //CURVEDETECT_IMAGE_H
