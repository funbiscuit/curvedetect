
#ifndef CURVEDETECT_IMAGE_H
#define CURVEDETECT_IMAGE_H

#include <string>
#include <vector>
#include "image_elements.h"

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

    int getPixelValue(int px, int py);

    /**
     * will snap point to closest black (0) point
     * so for more precision use with SnapToBary
     * @param ImagePoint
     */
    bool SnapToCurve(Vec2D& point, int binLevel, int dist);

    /**
     * will snap point to barycenter of current zoom region
     * will not be precise if there are grid lines in the region
     * so make sure that in zoom region only curve points are shown
     * @param ImagePoint
     */
    bool SnapToBary(Vec2D& point, int binLevel);


private:

    unsigned char* image;

    unsigned int texture;
    int width;
    int height;

    uint8_t* imagePixels;

    //used for storing result of getNearbyPoints
    std::vector<uint8_t> pixelsRegion;


    /**
     * Result will be stored in pixelsRegion variable
     * region will be a square with side = (2*hside+1)
     * @param px replaced with the local coordinates
     * @param py replaced with the local coordinates
     * @param hside half side of region
     * @return
     */
    bool getNearbyPoints(int& px, int& py, int hside);

};


#endif //CURVEDETECT_IMAGE_H
