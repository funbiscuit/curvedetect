
#include <snap_cache.h>

#include "snap_cache.h"

SnapCache::SnapCache()
{
    width = 0;
    height = 0;
    cachedBinLevel = -1;
}

bool SnapCache::isAvailable(int px, int py)
{
    return isInside(px, py) && snapCached[py*width + px];
}

void SnapCache::setBinLevel(int binLevel)
{
    if(cachedBinLevel != binLevel)
    {
        snapCached.clear(); //this to make sure all values are false
        snapCached.resize(width*height);
        cachedBinLevel = binLevel;
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

bool SnapCache::isInside(int px, int py)
{
    return px>=0 && py>=0 && px<width && py<height;
}

bool SnapCache::snap(int px, int py, Vec2D &point)
{
    if(isAvailable(px, py))
    {
        if(canSnap(px, py))
            point = snapTo[py*width + px];
        return canSnap(px, py);
    }
    return false;
}

bool SnapCache::canSnap(int px, int py)
{
    return snapTo[py*width + px].x>-1.f;
}

void SnapCache::cacheSnap(int px, int py, Vec2D pos, bool canSnap)
{
    if(isInside(px, py))
    {
        snapTo[py*width + px] = canSnap ? pos : Vec2D(-2.f, -2.f);
        snapCached[py*width + px] = true;
    }
}

