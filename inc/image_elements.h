
#ifndef CURVEDETECT_IMAGE_ELEMENTS_H
#define CURVEDETECT_IMAGE_ELEMENTS_H

#include <imgui.h>
#include <cstdint>


class Vec2D
{
public:
    double x=0.0;
    double y=0.0;

    Vec2D()
    = default;

    Vec2D(double x, double y)
    {
        this->x=x;
        this->y=y;
    }
    explicit Vec2D(const ImVec2& vec)
    {
        x=vec.x;
        y=vec.y;
    }
    ImVec2 ToImVec2() const
    {
        return ImVec2((float)x, (float)y);
    }
    inline double norm2()
    {
        return x*x + y*y;
    }
};
static inline bool operator==(const Vec2D& lhs, const Vec2D& rhs)  { return lhs.x==rhs.x && lhs.y==rhs.y; }
static inline Vec2D operator*(const Vec2D& lhs, const float rhs)   { return Vec2D(lhs.x*rhs, lhs.y*rhs); }
static inline Vec2D operator/(const Vec2D& lhs, const float rhs)   { return Vec2D(lhs.x/rhs, lhs.y/rhs); }
static inline Vec2D operator+(const Vec2D& lhs, const Vec2D& rhs)  { return Vec2D(lhs.x+rhs.x, lhs.y+rhs.y); }
static inline Vec2D operator-(const Vec2D& lhs, const Vec2D& rhs)  { return Vec2D(lhs.x-rhs.x, lhs.y-rhs.y); }
static inline Vec2D operator*(const Vec2D& lhs, const Vec2D& rhs)  { return Vec2D(lhs.x*rhs.x, lhs.y*rhs.y); }
static inline Vec2D operator/(const Vec2D& lhs, const Vec2D& rhs)  { return Vec2D(lhs.x/rhs.x, lhs.y/rhs.y); }
static inline Vec2D& operator+=(Vec2D& lhs, const Vec2D& rhs)      { lhs.x += rhs.x; lhs.y += rhs.y; return lhs; }
static inline Vec2D& operator-=(Vec2D& lhs, const Vec2D& rhs)      { lhs.x -= rhs.x; lhs.y -= rhs.y; return lhs; }
static inline Vec2D& operator*=(Vec2D& lhs, const float rhs)       { lhs.x *= rhs; lhs.y *= rhs; return lhs; }
static inline Vec2D& operator/=(Vec2D& lhs, const float rhs)       { lhs.x /= rhs; lhs.y /= rhs; return lhs; }


class ImageElement
{
private:
    static uint64_t nextId;
public:
    enum Type
    {
        POINT   = 1 << 0,
        X_TICK  = 1 << 1,
        Y_TICK  = 1 << 2,
        HORIZON = 1 << 3,

        TICKS = X_TICK | Y_TICK,
        ALL = POINT | TICKS | HORIZON
    };

    uint64_t id=nextId++;
    bool isSnapped=false;
    Vec2D imagePosition=Vec2D(0.0,0.0);

    ImageElement()
    = default;

    explicit ImageElement(const Vec2D& position)
    {
        imagePosition=position;
    }

    inline double X(){ return imagePosition.x; }
    inline double Y(){ return imagePosition.y; }
};

class ImagePoint : public ImageElement
{
public:
    bool isSubdivisionPoint = false;

    ImagePoint()
    = default;

    explicit ImagePoint(const Vec2D& position) : ImageElement(position)
    {

    }
};

class ImageTickLine : public ImageElement
{
public:
    double tickValue=0.0;
    std::string tickValueStr="0";
    ImageTickLine* backup=nullptr;

    ImageTickLine()
    = default;

    explicit ImageTickLine(const Vec2D& position) : ImageElement(position)
    {

    }

    void setValueStr(const char* str);
    static void filterValueStr(std::string& str, bool finalFilter);

    double DistTo(const Vec2D& imagePosition, const Vec2D& tickDirection);

    void MakeBackup();
    void RestoreBackup();
};


class ImageHorizon : public ImageElement
{
public:

    enum HorizonPoint //: int
    {
        NONE = 0,
        ORIGIN = 1,
        TARGET = 2,
    };

    ImageElement target;


    explicit ImageHorizon(const Vec2D& position) : ImageElement(position)
    {
        target.imagePosition=position;
        target.imagePosition.x+=100.0;
    }

    Vec2D VerticalDirection();
    Vec2D HorizontalDirection();
};


#endif //CURVEDETECT_IMAGE_ELEMENTS_H
