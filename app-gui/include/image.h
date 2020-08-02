
#ifndef CURVEDETECT_IMAGE_H
#define CURVEDETECT_IMAGE_H

#include <string>
#include <vector>

#include <QPixmap>

#include "image_elements.h"
#include "snap_cache.h"


class ImageData
{
public:
    ImageData() = default;
    ~ImageData();

    uint8_t* pixels=nullptr;
    int width;
    int height;
};


class Image {
public:
    Image(const std::string& path);
    Image(ImageData& imageData);
    ~Image();

    QPixmap getPixmap() const;

    int get_width();
    int get_height();
    bool is_loaded();
//    unsigned int get_texture() {
//        return texture;
//    }
    
    bool is_pixel_inside(int px, int py);

    bool snap(Vec2D &pos, int binLevel);

    bool set_curve_thickness(int thick);
    bool set_inverted(bool inverted);

private:
    std::vector<ImageData> images;
    std::vector<ImageData> images_inv;
    unsigned char* image = nullptr;

    QPixmap imagePix;
    
    //int snapDistance = 25;
    int snapMultiplier = 8; //snapDistance=snapMultiplier*curveThickness
    int curveThickness = 5;
    bool bInvertImage = false;
    
//    unsigned int texture;
    int width;
    int height;

    //used for storing result of getNearbyPoints
    std::vector<uint8_t> pixelsRegion;

    SnapCache snapCache;

    void generate_mipmaps();
    

    /**
     * Result will be stored in pixelsRegion variable
     * region will be a square with side = (2*hside+1)
     * @param px replaced with the local coordinates
     * @param py replaced with the local coordinates
     * @param hside half side of region
     * @return
     */
    bool update_pixel_region(int &px, int &py, int hside);

    /**
     * will snap point to closest black (0) point
     * so for more precision use with snap_to_bary
     * @param ImagePoint
     */
    bool snap_to_closest(Vec2D &point, int binLevel);

    /**
     * will snap point to barycenter of current zoom region
     * will not be precise if there are grid lines in the region
     * so make sure that in zoom region only curve points are shown
     * @param ImagePoint
     */
    bool snap_to_bary(Vec2D &point, int binLevel);
};


#endif //CURVEDETECT_IMAGE_H
