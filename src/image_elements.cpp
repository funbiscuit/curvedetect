


#include <cmath>
#include "image_elements.h"


uint64_t ImageElement::nextId=0;

double ImageTickLine::DistTo(const Vec2D& imagePosition, const Vec2D& tickDirection)
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


void ImageTickLine::MakeBackup()
{
    backup = new ImageTickLine(imagePosition);
    backup->tickValue = tickValue;
}

void ImageTickLine::RestoreBackup()
{
    if (backup == nullptr)
        return;

    imagePosition = backup->imagePosition;
    tickValue = backup->tickValue;
    delete backup;
    backup = nullptr;
}


Vec2D ImageHorizon::VerticalDirection()
{
    Vec2D dir = Vec2D(targetPosition.y - imagePosition.y, -targetPosition.x + imagePosition.x);
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

Vec2D ImageHorizon::HorizontalDirection()
{
    Vec2D dir = Vec2D(targetPosition.x - imagePosition.x, targetPosition.y - imagePosition.x);
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
