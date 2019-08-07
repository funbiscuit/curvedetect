
#include <image.h>
#include <iostream>
#include <chrono>

#include "image.h"
#include "snap_cache.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include <bitset>
#include <algorithm>

uint32_t bit_pos(uint32_t val)
{
    return std::bitset<32>(val-1).count();
}

// Round up to lower power of 2 (return x>>1 if it's already a power of 2)
inline int pow2floor (int x)
{
    if (x < 0)
        return 0;
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x=(x+1)>>1;
    x = x>0 ? x : 1;
    return x;
}



ImageData::~ImageData()
{
    if(pixels)
        delete[](pixels);
}

Image::Image(std::string path)
{
    texture=0;
    
    //create at most 10 mipmaps
    images.resize(10);

    image = stbi_load(path.c_str(), &width, &height, nullptr, 3);

    if(image)
    {
        images[0].pixels = new uint8_t[width*height];
        images[0].width = width;
        images[0].height = height;
        texture = ImGui_ImplOpenGL3_CreateTexture(image, width, height, false, false);

        snapCache.resize(width, height);
    
        for (int col = 0; col < width; col++)
        {
            for (int row = 0; row < height; row++)
            {
                float r = *(image + 3*(row * width + col));
                float g = *(image + 3*(row * width + col)+1);
                float b = *(image + 3*(row * width + col)+2);

                images[0].pixels[row * width + col]=uint8_t(0.2126f * r + 0.7152f * g + 0.0722f * b);
            }
        }

        auto start=std::chrono::high_resolution_clock::now();

        generate_mipmaps();
        
        auto end=std::chrono::high_resolution_clock::now();
        auto mseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "create took: " << mseconds << "\n";
    }
    else
        images.clear();




}

Image::Image(ImageData& imageData)
{
    if(!imageData.pixels)
        return;

    texture=0;

    //create at most 10 mipmaps
    images.resize(10);

    width=imageData.width;
    height=imageData.height;
    image = new uint8_t[width*height*3];
    memcpy(image, imageData.pixels, width*height*3);


    images[0].pixels = new uint8_t[width*height];
    images[0].width = width;
    images[0].height = height;
    texture = ImGui_ImplOpenGL3_CreateTexture(image, width, height, false, false);

    snapCache.resize(width, height);

    for (int col = 0; col < width; col++)
    {
        for (int row = 0; row < height; row++)
        {
            float r = *(image + 3*(row * width + col));
            float g = *(image + 3*(row * width + col)+1);
            float b = *(image + 3*(row * width + col)+2);

            images[0].pixels[row * width + col]=uint8_t(0.2126f * r + 0.7152f * g + 0.0722f * b);
        }
    }

    auto start=std::chrono::high_resolution_clock::now();

    generate_mipmaps();

    auto end=std::chrono::high_resolution_clock::now();
    auto mseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "create took: " << mseconds << "\n";


}

Image::~Image()
{
    if(image)
        delete(image);
    images.clear();
//    if(imagePixels)
//        delete(imagePixels);
    //TODO SIGSEGV if destroying after window close
//    if(texture!=0)
//        ImGui_ImplOpenGL3_DestroyTexture(texture);
}

void Image::generate_mipmaps()
{
    for(int i=1;i<images.size();++i)
    {
        int w1=images[i-1].width;
        int w2=w1/2;
        int h2=images[i-1].height/2;
        
        if(w2<10 || h2<10)
        {
            images.resize(i);
            break;
        }
        
        images[i].pixels = new uint8_t[w2*h2];
        images[i].width = w2;
        images[i].height = h2;
    
        for (int col = 0; col < w2; col++)
        {
            for (int row = 0; row < h2; row++)
            {
                uint8_t a = std::min(images[i-1].pixels[2*row*w1 + 2*col], images[i-1].pixels[2*row*w1+2*col+1]);
                uint8_t b = std::min(images[i-1].pixels[(2*row+1)*w1 + 2*col], images[i-1].pixels[(2*row+1)*w1+2*col+1]);
                images[i].pixels[row * w2 + col]=std::min(a, b);
            }
        }
        
    }
    
    
    
}


int Image::get_width()
{
    return image ? width : 0;
}

int Image::get_height()
{
    return image ? height : 0;
}

bool Image::is_loaded()
{
    return image!=nullptr;
}


bool Image::is_pixel_inside(int px, int py)
{
    if (!is_loaded() || px < 0 || py < 0)
        return false;
    
    return (px < width && py < height);
    
}

bool Image::update_pixel_region(int &px, int &py, int hside)
{
    
    if (!is_pixel_inside(px, py))
        return false;
    
    int localX, localY;
    localX = localY = hside;
    int side = hside*2+1;
    
    if (px < hside)
    {
        localX = px;
        px = hside;
    }
    if (py < hside)
    {
        localY = py;
        py = hside;
    }
    
    if (px >= width-hside)
    {
        localX = 2 * hside + 1 - width + px;
        px = width - hside - 1;
    }
    if (py >= height - hside)
    {
        localY = 2 * hside + 1 - height + py;
        py = height - hside - 1;
    }

    pixelsRegion.resize(side*side);
    
    for (int kx = px - hside; kx <= px + hside; kx++)
    {
        for (int ky = py - hside; ky <= py + hside; ky++)
        {
            pixelsRegion[(ky - py + hside)*side + kx - px + hside] = images[0].pixels[ky * width + kx];
        }
    }
    
    px = localX;
    py = localY;
    
    return true;
}

bool Image::snap(Vec2D &pos, int binLevel)
{
    //TODO dist is not changed in snap cache
    snapCache.set_bin_level(binLevel);
    int px = (int)pos.x;
    int py = (int)pos.y;

    if(snapCache.is_available(px, py))
        return snapCache.snap(px, py, pos);
    else
    {
        bool canSnap = snap_to_closest(pos, binLevel) && snap_to_bary(pos, binLevel);
        snapCache.cache_snap_info(px, py, pos, canSnap);
        return canSnap;
    }
}

void Image::set_curve_thickness(int thick)
{
    if(curveThickness==thick)
        return;
    curveThickness=thick;
    snapCache.set_curve_thick(curveThickness);
}

bool Image::snap_to_closest(Vec2D &point, int binLevel)
{
    if(!image)
        return false;

    int hside = snapMultiplier * curveThickness;
    int side = hside*2+1;

    if(width < side*2 || height<side*2)
        return false;

    int px = (int) std::round(point.x);
    int py = (int) std::round(point.y);
    
    if(!is_pixel_inside(px, py))
        return false;
    
    if(images[0].pixels[py * width + px] < binLevel)
        return true;

    int minDist = side*side;
    int closestX = -1;
    int closestY = -1;

    //determine pixel step based on curveThickness since then we will be averaging
    //over rectangle, we don't need to snap precisely
    int step = pow2floor(curveThickness)/2;
    step = step>0 ? step : 1;
    int image_i = bit_pos(step);

    for (int row = 0; row < side; row+=step)
    {
        for (int column = 0; column < side; column+=step)
        {
            int globalRow = py - hside + row;
            int globalColumn = px - hside + column;
            bool isBlack = false;
            if (globalRow >= 0 && globalRow < height && globalColumn >= 0 && globalColumn < width)
                isBlack = images[image_i].pixels[(globalRow/step) * (width/step) + globalColumn/step] < binLevel;

            if (isBlack)
            {
                int dx = hside - column-2;
                int dy = hside - row-2;
                int dist = dx * dx + dy * dy;
                if (dist < minDist)
                {
                    minDist = dist;
                    closestX = column+2;
                    closestY = row+2;
                }
            }
        }
    }

    if (closestX == -1 || closestY == -1)
        return false;

    point.x = px + closestX - hside;
    point.y = py + closestY - hside;

    return true;
}

bool Image::snap_to_bary(Vec2D &point, int binLevel)
{
    int localX, localY;//to this vars local coords will be written

    //TODO move to settings
    int hside = curveThickness;// ZoomPixelHSide;
    int side = hside*2+1;

    localX = int(point.x);
    localY = int(point.y);

    if (!update_pixel_region(localX, localY, hside))
    {
        std::cout << "Bad point!\n";
        return false;
    }

    int baryMass = 0;
    Vec2D BaryOffset;

    for (int kx = 0; kx <= 2 * hside; kx++)
    {
        for (int ky = 0; ky <= 2 * hside; ky++)
        {
            auto col = pixelsRegion[ky*side + kx];
            if (col < binLevel)
            {
                baryMass += binLevel - col;
                BaryOffset += Vec2D(double(kx - localX), double(ky - localY))*double(binLevel - col);
            }
        }
    }

    if (baryMass > 0)
    {
        BaryOffset /= double(baryMass);
        point += BaryOffset;
        return true;
    } else
        return false;
}
