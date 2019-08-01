
#include <image.h>
#include <iostream>

#include "image.h"
#include "snap_cache.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"


Image::Image(std::string path)
{
    texture=0;
    imagePixels = nullptr;

    image = stbi_load(path.c_str(), &width, &height, nullptr, 3);

    if(image)
    {
        texture = ImGui_ImplOpenGL3_CreateTexture(image, width, height, false, false);

        imagePixels = new uint8_t[width*height];
        snapCache.resize(width, height);
    
        for (int col = 0; col < width; col++)
        {
            for (int row = 0; row < height; row++)
            {
                float r = *(image + 3*(row * width + col));
                float g = *(image + 3*(row * width + col)+1);
                float b = *(image + 3*(row * width + col)+2);

                imagePixels[row * width + col]=uint8_t(0.2126f * r + 0.7152f * g + 0.0722f * b);
            }
        }
    }
}

Image::~Image()
{
    if(image)
        delete(image);
    if(imagePixels)
        delete(imagePixels);
    //TODO SIGSEGV if destroying after window close
//    if(texture!=0)
//        ImGui_ImplOpenGL3_DestroyTexture(texture);
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


bool Image::isPixelInside(int px, int py)
{
    if (!is_loaded() || px < 0 || py < 0)
        return false;
    
    return (px < width && py < height);
    
}


int Image::getPixelValue(int px, int py)
{
    if (!isPixelInside(px, py))
        return 0;
    
    return imagePixels[py * width + px];
}


bool Image::getNearbyPoints(int& px, int& py, int hside)
{
    
    if (!isPixelInside(px, py))
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
            pixelsRegion[(ky - py + hside)*side + kx - px + hside] = imagePixels[ky * width + kx];
        }
    }
    
    px = localX;
    py = localY;
    
    return true;
}

bool Image::Snap(Vec2D& pos, int binLevel, int dist)
{
    //TODO dist is not changed in snap cache
    snapCache.setBinLevel(binLevel);
    int px = (int)pos.x;
    int py = (int)pos.y;

    if(snapCache.isAvailable(px, py))
        return snapCache.snap(px, py, pos);
    else
    {
        bool canSnap = SnapToCurve(pos, binLevel, dist) && SnapToBary(pos, binLevel);
        snapCache.cacheSnap(px, py, pos, canSnap);
        return canSnap;
    }
}

bool Image::SnapToCurve(Vec2D& point, int binLevel, int dist)
{
    if(!image)
        return false;

    int hside = dist;
    int side = hside*2+1;

    if(width < side*2 || height<side*2)
        return false;

    int px = (int) std::round(point.x);
    int py = (int) std::round(point.y);

    int minDist = side*side;
    int closestX = -1;
    int closestY = -1;


    for (int row = 0; row < side; ++row)
    {
        for (int column = 0; column < side; ++column)
        {
            int globalRow = py - hside + row;
            int globalColumn = px - hside + column;
            bool isBlack = false;
            if (globalRow >= 0 && globalRow < height && globalColumn >= 0 && globalColumn < width)
                isBlack = imagePixels[globalRow * width + globalColumn] < binLevel;

            if (isBlack)
            {
                int dx = hside - column;
                int dy = hside - row;
                int dist = dx * dx + dy * dy;
                if (dist < minDist)
                {
                    minDist = dist;
                    closestX = column;
                    closestY = row;
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

bool Image::SnapToBary(Vec2D& point, int binLevel)
{
    int localX, localY;//to this vars local coords will be written

    int baryX, baryY; //local coords of snapped point

    //TODO move to settings
    int hside = 6;// ZoomPixelHSide;
    int side = hside*2+1;

    localX = int(point.x);
    localY = int(point.y);

    if (!getNearbyPoints(localX, localY, hside))
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
