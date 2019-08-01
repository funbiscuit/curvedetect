
#ifndef CURVEDETECT_SNAP_CACHE_H
#define CURVEDETECT_SNAP_CACHE_H

#include "image_elements.h"
#include <vector>

/**
 * cache is used to quickly calculate where to snap certain point
 */
class SnapCache
{
public:
    SnapCache();
    bool isAvailable(int px, int py);

    void setBinLevel(int binLevel);

    bool snap(int px, int py, Vec2D& point);

    bool canSnap(int px, int py);

    void cacheSnap(int px, int py, Vec2D pos, bool canSnap);

    void resize(int width, int height);

private:

    bool isInside(int px, int py);

    //holds positions where we should snap to
    //negative x or y means thath snap is cached but there is nowhere to snap
    std::vector<Vec2D> snapTo;

    //whether this position has valid cache
    std::vector<bool> snapCached;

    int cachedBinLevel;

    int width;
    int height;

};


#endif //CURVEDETECT_SNAP_CACHE_H
