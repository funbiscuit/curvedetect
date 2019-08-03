
#include <snap_cache.h>
#include <iostream>

#include "snap_cache.h"

SnapCache::SnapCache()
{
    width = 0;
    height = 0;
    cachedBinLevel = -1;
    cachedCurveThick = -1;
}

bool SnapCache::is_available(int px, int py)
{
    return is_inside(px, py) && snapCached[py*width + px];
}

void SnapCache::set_bin_level(int binLevel)
{
    if(cachedBinLevel != binLevel)
    {
        snapCached.clear(); //this to make sure all values are false
        snapCached.resize(width*height);
        cachedBinLevel = binLevel;
    }
}
void SnapCache::set_curve_thick(int thick)
{
    if(cachedCurveThick != thick)
    {
        snapCached.clear(); //this to make sure all values are false
        snapCached.resize(width*height);
        cachedCurveThick = thick;
    }
}

void SnapCache::resize(int width, int height)
{
    snapTo.resize(width*height);
    snapCached.clear(); //this to make sure all values are false
    snapCached.resize(width*height);
    this->width = width;
    this->height = height;
}

bool SnapCache::is_inside(int px, int py)
{
    return px>=0 && py>=0 && px<width && py<height;
}

bool SnapCache::snap(int px, int py, Vec2D &point)
{
    if(is_available(px, py))
    {
        if(can_snap(px, py))
            point = snapTo[py*width + px];
        return can_snap(px, py);
    }
    return false;
}

bool SnapCache::can_snap(int px, int py)
{
    return snapTo[py*width + px].x>-1.f;
}

void SnapCache::cache_snap_info(int px, int py, Vec2D pos, bool canSnap)
{
    if(is_inside(px, py))
    {
        snapTo[py*width + px] = canSnap ? pos : Vec2D(-2.f, -2.f);
        snapCached[py*width + px] = true;
    }
}

