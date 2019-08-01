


#include <cmath>
#include <cstdlib>
#include <iostream>
#include <image_elements.h>
#include <regex>

#include "image_elements.h"


//id 1 and 2 are reserved for horizon and 0 is none
uint64_t ImageElement::nextId=3;


void ImageTickLine::set_value(const char *str)
{
    char* end;
    auto val = strtod(str, &end);
    if(! *end)
    {
        tickValueStr = str;
        tickValue=val;
    }
}

void ImageTickLine::filter_value(std::string &str, bool finalFilter)
{
    std::regex regInvalidChar("[^,.0-9-]");
    str=std::regex_replace(str, regInvalidChar, "");
    std::replace(str.begin(), str.end(), ',', '.');

    bool dot=false;
    bool min=str.empty() ? true : str[0]!='-';

    auto it=str.begin();
    while(it!=str.end())
    {
        if(*it=='-' && min)
            it=str.erase(it);
        else if(*it=='.')
        {
            if(dot)
                it=str.erase(it);
            else
            {
                dot = true;
                ++it;
            }
        }
        else
            ++it;
        min=true;
    }
    while(finalFilter && !str.empty() &&
    (str.back()=='.' || str.back()=='-' || (dot && str.back()=='0')))
    {
        if(str.back()=='.')
            dot=false;
        str.pop_back();
    }
    while(str.length()>1 && str.front()=='0')
        str.erase(str.begin());
}


double ImageTickLine::dist_to(const Vec2D &imagePosition, const Vec2D &tickDirection)
{
    double dx = tickDirection.x;
    double dy = tickDirection.y;

    double norm = std::sqrt(dx * dx + dy * dy);
    if (norm < 0.1)
        return -1.0;

    Vec2D thisPos = this->imagePosition;

    //A and B are in pixels
    double normDy = dy / norm;
    double normDx = dx / norm;
    double extraShift = -normDy * thisPos.x + normDx * thisPos.y;
    // C2=B*point1.x-A*point1.y

    //x line: A*x+B*y+C=0		C=-A*x0-B*y0
    //y line: -B*x+A*y+C2=0;	C2=B*x0-A*y0

    return std::abs(normDy * imagePosition.x - normDx * imagePosition.y + extraShift);
}


void ImageTickLine::create_backup()
{
    backup = new ImageTickLine(imagePosition);
    backup->tickValue = tickValue;
}

void ImageTickLine::restore_backup()
{
    if (backup == nullptr)
        return;

    imagePosition = backup->imagePosition;
    tickValue = backup->tickValue;
    delete backup;
    backup = nullptr;
}


Vec2D ImageHorizon::vertical_dir()
{
    Vec2D dir = Vec2D(target.imagePosition.y - imagePosition.y, -target.imagePosition.x + imagePosition.x);
    double norm = std::sqrt(dir.x * dir.x + dir.y * dir.y);

    if (norm > 2.0)
    {
        dir.x /= norm;
        dir.y /= norm;
    }
    else
    {
        dir.x = 0.0;
        dir.y = -1.0;
    }

    return dir;
}

Vec2D ImageHorizon::horizontal_dir()
{
    Vec2D dir = Vec2D(target.imagePosition.x - imagePosition.x, target.imagePosition.y - imagePosition.y);
    double norm = std::sqrt(dir.x * dir.x + dir.y * dir.y);

    if (norm > 2.0)
    {
        dir.x /= norm;
        dir.y /= norm;
    }
    else
    {
        dir.x = 1.0;
        dir.y = 0.0;
    }

    return dir;
}
