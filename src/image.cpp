
#include <image.h>
#include <iostream>

#include "image.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "imgui.h"
#include "imgui_impl_opengl3.h"


Image::Image(std::string path)
{
    texture=0;

    image = stbi_load(path.c_str(), &width, &height, nullptr, 3);

    if(image)
    {
        texture = ImGui_ImplOpenGL3_CreateTexture(image, width, height, false, false);
    
        ImageMatrix = MatrixXi(height, width);
    
        for (int kx = 0; kx < width; kx++)
        {
            for (int ky = 0; ky < height; ky++)
            {
                float r = *(image + 3*(ky * width + kx));
                float g = *(image + 3*(ky * width + kx + 1));
                float b = *(image + 3*(ky * width + kx + 2));
            
                ImageMatrix(ky, kx) =int(0.2126f * r + 0.7152f * g + 0.0722f * b);
            }
        }
    }
}

Image::~Image()
{
    if(image)
        delete(image);
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
    
    return ImageMatrix(py, px);
}


bool Image::getNearbyPoints(int& px, int& py, int hside, MatrixXi& out_PointRegion)
{
    
    if (!isPixelInside(px, py))
        return false;
    
    int localX, localY;
    localX = localY = hside;
    
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
    
    out_PointRegion.resize(2 * hside + 1, 2 * hside + 1);
    
    for (int kx = px - hside; kx <= px + hside; kx++)
    {
        for (int ky = py - hside; ky <= py + hside; ky++)
        {
            out_PointRegion(ky - py + hside, kx - px + hside) = ImageMatrix(ky, kx);
        }
    }
    
    px = localX;
    py = localY;
    
    return true;
}

bool Image::getClosestBlack(int& px, int& py, int hside, int ColorLevel)
{
    if (!isPixelInside(px, py))
        return false;
    
    for (int side = 0; side <= hside; side++)
    {
        int kx, ky;
        
        int left = (px < side ? 0 : px - side);
        int right = (px > width - 1 - side ? width - 1 - px : px + side);
        int top = (py < side ? 0 : py - side);
        int bottom = (py > height - 1 - side ? height - 1 - px : py + side);
        
        for (kx = left; kx <= right; kx++)
        {
            for (ky = top; ky <= bottom; ky++)
            {
                int dist2 = (kx - px)*(kx - px) + (ky - py)*(ky - py);
                
                if(dist2> side*side || dist2<(side-1)*(side-1))
                    continue;
                
                if (ImageMatrix(ky, kx) < ColorLevel)
                {
                    px = kx;
                    py = ky;
                    return true;
                }
            }
// 			if (ImageMatrix(top, kx) < CurveColorLevel)
// 			{
// 				px = kx;
// 				py = top;
// 				return true;
// 			}
// 			if (ImageMatrix(bottom, kx) < CurveColorLevel)
// 			{
// 				px = kx;
// 				py = bottom;
// 				return true;
// 			}
        }
// 		for (ky = top; ky <= bottom; ky++)
// 		{
// 			if (ImageMatrix(ky, left) < CurveColorLevel)
// 			{
// 				px = left;
// 				py = ky;
// 				return true;
// 			}
// 			if (ImageMatrix(ky, right) < CurveColorLevel)
// 			{
// 				px = right;
// 				py = ky;
// 				return true;
// 			}
// 		}
    
    
    
    }
    
    return false;
}
