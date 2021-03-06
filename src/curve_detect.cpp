#include "curve_detect.h"


#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

#include <algorithm>
#include <cmath>
#include <curve_detect.h>

#include "mat_file_writer.h"
#include "main_app.h"



CurveDetect::CurveDetect(std::shared_ptr<Image> image) : horizon(Vec2D(100,100))
{
    this->image=image;

    binLevel = 127;

    subdivLevel = 3;
}

uint64_t CurveDetect::get_hovered_id(int selectionFilter)
{
    if( (selectionFilter & ImageElement::POINT) && hoveredPoint )
        return hoveredPoint;
    if( (selectionFilter & ImageElement::X_TICK) && hoveredXtick )
        return hoveredXtick;
    if( (selectionFilter & ImageElement::Y_TICK) && hoveredYtick )
        return hoveredYtick;
    if( (selectionFilter & ImageElement::HORIZON) && hoveredOrigin )
        return hoveredOrigin;
    return 0;
}

void CurveDetect::update_hovered(Vec2D hoveredImagePos)
{
//    auto prevHovered=GetHoveredId(ImageElement::ALL);

    update_hovered_point(hoveredImagePos);
    update_hovered_tickx(hoveredImagePos);
    update_hovered_ticky(hoveredImagePos);
    update_hovered_horizon(hoveredImagePos);

//    auto newHovered = GetHoveredId(ImageElement::ALL);
}

void CurveDetect::update_hovered_point(Vec2D imagePos)
{
    hoveredPoint = 0;
    double minDist = hoverZone * hoverZone;

    for (auto& point : userPoints)
    {
        double dist = (point.imagePosition - imagePos).norm2();

        if(dist<minDist)
        {
            minDist = dist;
            hoveredPoint = point.id;
        }
    }
}

void CurveDetect::update_hovered_horizon(Vec2D imagePos)
{
    hoveredOrigin = ImageHorizon::NONE;
    double minDist = hoverZone * hoverZone;

    double dist1 = (horizon.imagePosition - imagePos).norm2();
    double dist2 = (horizon.target.imagePosition - imagePos).norm2();

    if (dist1 < dist2 && dist1 < minDist)
        hoveredOrigin = ImageHorizon::ORIGIN;
    else if (dist2 < minDist)
        hoveredOrigin = ImageHorizon::TARGET;
}

void CurveDetect::update_hovered_tickx(Vec2D imagePos)
{
    hoveredXtick = 0;
    double minDist = hoverZone;
    Vec2D tickDir = horizon.vertical_dir();

    for (auto& tick : xticks)
    {
        double dist = tick.dist_to(imagePos, tickDir);

        if(dist<minDist)
        {
            minDist = dist;
            hoveredXtick = tick.id;
        }
    }
}

void CurveDetect::update_hovered_ticky(Vec2D imagePos)
{
    hoveredYtick = 0;
    double minDist = hoverZone;
    Vec2D tickDir = horizon.horizontal_dir();

    for (auto& tick : yticks)
    {
        double dist = tick.dist_to(imagePos, tickDir);

        if(dist<minDist)
        {
            minDist = dist;
            hoveredYtick = tick.id;
        }
    }
}

bool CurveDetect::is_export_ready(int &out_Result)
{
    out_Result = READY;

    if (userPoints.size() < 2)
        out_Result |= NO_POINTS;

    auto dir1=horizon.target.imagePosition-horizon.imagePosition;
    double norm = std::sqrt(dir1.x*dir1.x + dir1.y*dir1.y);

    auto dxtick=xticks[0].imagePosition-xticks[1].imagePosition;

    //for deriving a and b
    double det1 = (dxtick.x*dir1.x + dxtick.y*dir1.y)/norm;

    if (std::abs(det1) < minTickPixelDiff)
        out_Result |= PIXEL_OVERLAP_X_GRID;
    else if (xticks[0].tickValue == xticks[1].tickValue)
        out_Result |= VALUE_OVERLAP_X_GRID;


    auto dytick=yticks[0].imagePosition-yticks[1].imagePosition;

    //for deriving c and d
    double det2 = (-dytick.x*dir1.y + dytick.y*dir1.x)/norm;

    if (std::abs(det2)<minTickPixelDiff)
        out_Result |= PIXEL_OVERLAP_Y_GRID;
    else if (yticks[0].tickValue == yticks[1].tickValue)
        out_Result |= VALUE_OVERLAP_Y_GRID;

    return out_Result == READY;
}

std::string CurveDetect::get_points_text(std::string columnSeparator,
                                         std::string lineEnding, char decimalSeparator)
{
    int out_Result;
    if (!is_export_ready(out_Result))
        return "";

    auto bundle = get_points_bundle();

    std::stringstream sstr;

    std::string nums = "";

    std::string tempColumnSeparator = columnSeparator;
    std::string tempLineEnding = lineEnding;

    //new line sequence that is needed by default notepad
    const char newLineSeq[3] = { char(0x0d), char(0x0a), '\0' };

    for (size_t i = 0; i<tempColumnSeparator.size(); i++)
    {
        if (tempColumnSeparator[i] == '\n')
        {
            tempColumnSeparator.replace(i, i + 1, newLineSeq);
            i++;//since we added one extra char
        }
    }

    for (size_t i = 0; i<tempLineEnding.size(); i++)
    {
        if (tempLineEnding[i] == '\n')
        {
            tempLineEnding.replace(i, i + 1, newLineSeq);
            i++;//since we added one extra char
        }
    }

    for (int kp = 0; kp < bundle->allNum; ++kp)
    {
        double_to_string(bundle->allPointsReal[kp*2], decimalSeparator, nums);

        sstr << nums;
        sstr << tempColumnSeparator;

        double_to_string(bundle->allPointsReal[kp*2+1], decimalSeparator, nums);
        sstr<< nums;
        sstr << tempLineEnding;
    }

    return sstr.str();
}

void CurveDetect::double_to_string(double num, char decimalSeparator, std::string &out_String)
{
    out_String = std::to_string(num);

    if (decimalSeparator == '.')
        std::replace(out_String.begin(), out_String.end(), ',', '.');
    else if (decimalSeparator == ',')
        std::replace(out_String.begin(), out_String.end(), '.', ',');

    //std::cout << out_String << "\n";
}

std::shared_ptr<PointsBundle> CurveDetect::get_points_bundle()
{
    auto res=std::make_shared<PointsBundle>();

    update_subdiv(true);

    auto Nu=(int) userPoints.size();

    //we should have at least two user points (although it should be already checked)
    if(Nu<2)
        return res;

    int Na=1;//we need to count begin point of first segment

    for(auto& segment : segments)
        Na+=(int) segment.points.size()-1;


    res->userPointsPixels=new double[Nu*2];
    res->userPointsReal=new double[Nu*2];
    res->allPointsPixels=new double[Na*2];
    res->allPointsReal=new double[Na*2];


    int i=2;//we counted first point outside main loop
    res->allPointsPixels[0]=segments[0].begin.imagePosition.x;
    res->allPointsPixels[1]=segments[0].begin.imagePosition.y;

    std::map<std::pair<double, double>, bool> included;

    for(auto& segment : segments)
    {
        //count all subdiv points and end point (begin point of each segmented is not counted
        //since its the same as the end point of the previous segment. thats why begin point of
        //first segment was counted outside loop
        for(size_t j=1; j<segment.points.size(); ++j)
        {
            const auto& point = segment.points[j].imagePosition;

            auto key = std::pair<double, double>(point.x, point.y);
            if(included.find(key)!=included.end())
            {
                //there was a point with the same pixel coordinates - skip it
                --Na;
                continue;
            }

            res->allPointsPixels[i]=point.x;
            res->allPointsPixels[i+1]=point.y;
            auto real=image_point_to_real(point);
            res->allPointsReal[i]=real.x;
            res->allPointsReal[i+1]=real.y;

            included.insert(std::pair<std::pair<double, double>, bool>(key, true));
            i+=2;
        }
    }

    for (int i = 0; i < Nu; ++i)
    {
        res->userPointsPixels[2*i]=userPoints[i].imagePosition.x;
        res->userPointsPixels[2*i+1]=userPoints[i].imagePosition.y;
        auto real=image_point_to_real(userPoints[i].imagePosition);
        res->userPointsReal[2*i]=real.x;
        res->userPointsReal[2*i+1]=real.y;
    }
    res->allNum=Na;
    res->userNum=Nu;

    return res;
}

void CurveDetect::export_points_mat_file(const char *path)
{
    auto bundle = get_points_bundle();

    auto writer = MatFileWriter::get(path);

    if (writer)
    {
        writer->matrix("UserPointsPixels", bundle->userPointsPixels, bundle->userNum, 2)
                .matrix("SubdividedPointsPixels", bundle->allPointsPixels, bundle->allNum, 2)
                .matrix("UserPointsReal", bundle->userPointsReal, bundle->userNum, 2)
                .matrix("SubdividedPointsReal", bundle->allPointsReal, bundle->allNum, 2)
                .close();
    }
}


void CurveDetect::sort_points()
{
    auto origin = horizon.imagePosition;
    auto target = horizon.target.imagePosition;

    std::sort(userPoints.begin(), userPoints.end(), [&origin, &target](const ImagePoint &lhs, const ImagePoint &rhs)
    {
        double proj_i = (target.x - origin.x)*(lhs.imagePosition.x - origin.x) + (target.y - origin.y)*(lhs.imagePosition.y - origin.y);
        double proj_j = (target.x - origin.x)*(rhs.imagePosition.x - origin.x) + (target.y - origin.y)*(rhs.imagePosition.y - origin.y);

        return proj_i<proj_j;
    });
}

bool CurveDetect::are_points_sorted()
{
    auto origin = horizon.imagePosition;
    auto target = horizon.target.imagePosition;

    return std::is_sorted(userPoints.begin(), userPoints.end(), [&origin, &target](const ImagePoint &lhs, const ImagePoint &rhs)
    {
        double proj_i = (target.x - origin.x)*(lhs.imagePosition.x - origin.x) + (target.y - origin.y)*(lhs.imagePosition.y - origin.y);
        double proj_j = (target.x - origin.x)*(rhs.imagePosition.x - origin.x) + (target.y - origin.y)*(rhs.imagePosition.y - origin.y);

        return proj_i<proj_j;
    });
}

void CurveDetect::update_subdiv(bool fullUpdate)
{
    //when fast update is true then only intervals where end positions change will be updated
    //everything else should be the same so we don't bother updating them
    bool isSorted = are_points_sorted();
    bool fastUpdate = !fullUpdate && isSorted;

    auto userPointsCount = (int) userPoints.size();

    if (userPointsCount < 2)
    {
        segments.clear();
        return;
    }
    if(!isSorted)
        sort_points();


    if (segments.size() != userPointsCount-1)
    {
        segments.clear();
        segments.resize(userPointsCount-1);
        fastUpdate=false;
    }

    //subdiv is used on the whole curve. So two curves with same length but different number
    //of user points will have about the same total number of points (if subdiv level is the same)
    double totalDist=0;
    for (int i = 0; i < userPointsCount-1; ++i)
        totalDist+=(userPoints[i].imagePosition-userPoints[i+1].imagePosition).norm();

    int minStep=int(totalDist)>>subdivLevel;
    if(minStep<1)
        minStep=1;

    bool forceSubdiv = false;
    //each segment (line between user points) will be subdivided with Ni points
    //Ni is calculated based on distance between points. Bigger distance - more points are needed
    for (int i = 0; i < userPointsCount-1; ++i)
    {
        const auto& leftPosNew = userPoints[i];
        const auto& rightPosNew = userPoints[i+1];
        Vec2D leftPos = leftPosNew.imagePosition;
        Vec2D rightPos = rightPosNew.imagePosition;

        //check that both ends didnt move (if it is allowed to check this)
        if(!forceSubdiv && fastUpdate
           && leftPos == segments[i].begin.imagePosition
           && rightPos == segments[i].end.imagePosition)
            continue;
        else
        {
            forceSubdiv = !(rightPos == segments[i].end.imagePosition);
        }

        segments[i].begin=leftPosNew;
        segments[i].end=rightPosNew;

        double dist = (leftPos-rightPos).norm();
        int count = (int)(dist/minStep);
        if(count<1)
            count=1;
        int left = 0;
        int right = count;
        int extraPointsHalf = (count-1)/2;

        segments[i].points.resize(count+1);

        segments[i].points[0]=leftPosNew;
        segments[i].points[count]=rightPosNew;

        for(int j=0; j<= extraPointsHalf; ++j)
        {
            leftPos = segments[i].points[left].imagePosition;
            rightPos = segments[i].points[right].imagePosition;

            auto& nextLeft = segments[i].points[left+1];

            nextLeft.imagePosition = leftPos + (rightPos-leftPos)/double(right-left);
            nextLeft.isSnapped = snap(nextLeft.imagePosition);
            nextLeft.isSubdivisionPoint = true;

            if(j!=extraPointsHalf || count%2!=0)
            {
                auto& nextRight = segments[i].points[right-1];
                nextRight.imagePosition = rightPos - (rightPos-leftPos)/double(right-left);
                nextRight.isSnapped = snap(nextRight.imagePosition);
                nextRight.isSubdivisionPoint = true;
            }

            ++left;
            --right;
        }
        //TODO should use the same input data and not alter it during loop
        for(int j=1; j < count; ++j)
        {
            segments[i].points[j].imagePosition+=(segments[i].points[j-1].imagePosition+segments[i].points[j+1].imagePosition);
            segments[i].points[j].imagePosition/=3;
        }
        segments[i].points[0]=leftPosNew;
        segments[i].points[count]=rightPosNew;
    }
}

bool CurveDetect::snap(Vec2D &pos)
{
    return image ? image->snap(pos, binLevel) : false;
}

Vec2D CurveDetect::image_point_to_real(const Vec2D &point)
{
    Vec2D RealPoint;

    //this function should be called after check that
    //we can really calculate real points
    //so all x and y ticks are defined

    Vec2D Scale, Offset;

    //if scale is not linear, we will need to change tick values so it is linear and then convert point
    double x0=xticks[0].tickValue, x1=xticks[1].tickValue, y0=yticks[0].tickValue, y1=yticks[1].tickValue;

    if(xscale==LOG)
    {
        x0=std::log(x0);
        x1=std::log(x1);
    }

    if(yscale==LOG)
    {
        y0=std::log(y0);
        y1=std::log(y1);
    }

    auto dir1=horizon.target.imagePosition-horizon.imagePosition;
    auto dxtick=xticks[0].imagePosition-xticks[1].imagePosition;
    auto dytick=yticks[0].imagePosition-yticks[1].imagePosition;

    Scale.x = (x0 - x1) / dxtick.x;
    Scale.y = (y0 - y1) / dytick.y;

    Offset.x = x1 - xticks[1].X()*Scale.x;
    Offset.y = y1 - yticks[1].Y()*Scale.y;

    //xticks[1].z + (ImagePoint.x- xticks[1].x)*Scale.x

    RealPoint.x = Offset.x + point.x*Scale.x;
    RealPoint.y = Offset.y + point.y*Scale.y;

    //for deriving a and b
    double det1 = dxtick.x*dir1.x + dxtick.y*dir1.y;

    //for deriving c and d
    double det2 = -dytick.x*dir1.y + dytick.y*dir1.x;

    double a, b, c, d, e, f;

    a =  (x0 - x1)*dir1.x / det1;
    b =  (x0 - x1)*dir1.y / det1;

    c = -(y0 - y1)*dir1.y / det2;
    d =  (y0 - y1)*dir1.x / det2;

    e = x0 - a*xticks[0].X() - b*xticks[0].Y();
    f = y0 - c*yticks[0].X() - d*yticks[0].Y();

    RealPoint.x = a*point.x + b*point.y + e;
    RealPoint.y = c*point.x + d*point.y + f;

    if(xscale==LOG)
        RealPoint.x=std::exp(RealPoint.x);

    if(yscale==LOG)
        RealPoint.y=std::exp(RealPoint.y);


    return RealPoint;
}

bool CurveDetect::can_reset()
{
    if(!userPoints.empty())
        return true;

    if(xticks[0].tickValueStr!="0" || xticks[1].tickValueStr!="1")
        return true;
    if(yticks[0].tickValueStr!="0" || yticks[1].tickValueStr!="1")
        return true;

    if (image)
    {
        if(xticks[0].imagePosition.x != image->get_width()*0.2)
            return true;
        if(xticks[0].imagePosition.y != image->get_height()*0.5)
            return true;
        if(xticks[1].imagePosition.x != image->get_width()*0.8)
            return true;
        if(xticks[1].imagePosition.y != image->get_height()*0.5)
            return true;

        if(yticks[0].imagePosition.x != image->get_width()*0.5)
            return true;
        if(yticks[0].imagePosition.y != image->get_height()*0.8)
            return true;
        if(yticks[1].imagePosition.x != image->get_width()*0.5)
            return true;
        if(yticks[1].imagePosition.y != image->get_height()*0.2)
            return true;
    }

    return can_reset_horizon();
}

void CurveDetect::reset_all()
{
    userPoints.clear();
    segments.clear();

    xticks.resize(2);
    yticks.resize(2);

    xticks[0].set_value("0");
    xticks[1].set_value("1");
    yticks[0].set_value("0");
    yticks[1].set_value("1");

    if (image)
    {
        xticks[0].imagePosition.x = image->get_width()*0.2;
        xticks[0].imagePosition.y = image->get_height()*0.5;
        xticks[1].imagePosition.x = image->get_width()*0.8;
        xticks[1].imagePosition.y = image->get_height()*0.5;

        yticks[0].imagePosition.x = image->get_width()*0.5;
        yticks[0].imagePosition.y = image->get_height()*0.8;
        yticks[1].imagePosition.x = image->get_width()*0.5;
        yticks[1].imagePosition.y = image->get_height()*0.2;
    }

    deselect_all();

    reset_horizon();

}
void CurveDetect::reset_horizon()
{
    selectedOrigin = ImageHorizon::NONE;
    hoveredOrigin = ImageHorizon::NONE;
    horizon.target.imagePosition=horizon.imagePosition + Vec2D(100.0, 0.0);

    if (image)
    {
        horizon.imagePosition.x=image->get_width()*0.1;
        horizon.imagePosition.y=image->get_height()*0.5;
        horizon.target.imagePosition.x=image->get_width()*0.9;
        horizon.target.imagePosition.y=image->get_height()*0.5;
    }
}

bool CurveDetect::can_reset_horizon()
{
    if (image)
    {
        if(horizon.imagePosition.x!=image->get_width()*0.1)
            return true;
        if(horizon.imagePosition.y!=image->get_height()*0.5)
            return true;
        if(horizon.target.imagePosition.x!=image->get_width()*0.9)
            return true;
        if(horizon.target.imagePosition.y!=image->get_height()*0.5)
            return true;
    } else
    {
        if(horizon.target.imagePosition.x!=horizon.imagePosition.x + 100.0)
            return true;
        if(horizon.target.imagePosition.y!=horizon.imagePosition.y)
            return true;
    }

    return false;
}

void CurveDetect::snap_selected()
{
    auto sel = get_selected();

    if(sel && image)
        sel->isSnapped = snap(sel->imagePosition);
}

void CurveDetect::backup_selected_tick()
{
    auto sel = (ImageTickLine*) get_selected();
    if(sel != nullptr)
        sel->create_backup();
}

void CurveDetect::add_point(Vec2D pos)
{
    userPoints.emplace_back(pos);
    selectedPoint = userPoints.back().id;
    snap_selected();
    update_subdiv(true);
}

bool CurveDetect::select_hovered(int selectionFilter)
{
    if( (selectionFilter & ImageElement::POINT) && hoveredPoint )
    {
        deselect_all();
        selectedPoint = hoveredPoint;
        return true;
    }
    if( (selectionFilter & ImageElement::X_TICK) && hoveredXtick )
    {
        deselect_all();
        selectedXtick = hoveredXtick;
        return true;
    }
    if( (selectionFilter & ImageElement::Y_TICK) && hoveredYtick )
    {
        deselect_all();
        selectedYtick = hoveredYtick;
        return true;
    }
    if( (selectionFilter & ImageElement::HORIZON) && hoveredOrigin )
    {
        deselect_all();
        selectedOrigin = hoveredOrigin;
        return true;
    }

    return false;
}

ImageElement* CurveDetect::get_selected()
{
    if(selectedPoint)
    {
        for (auto &point : userPoints)
            if (point.id == selectedPoint)
                return &point;
    }
    else if(selectedXtick)
    {
        for (auto &tick : xticks)
            if(tick.id == selectedXtick)
                return &tick;
    }
    else if(selectedYtick)
    {
        for (auto &tick : yticks)
            if(tick.id == selectedYtick)
                return &tick;
    }
    else if(selectedOrigin)
    {
        if(selectedOrigin == ImageHorizon::ORIGIN)
            return &horizon;
        else
            return &horizon.target;
    }
    return nullptr;
}

void CurveDetect::delete_selected()
{
    if(selectedPoint)
    {
        for(auto it=userPoints.begin();it!=userPoints.end();++it)
        {
            if(it->id == selectedPoint)
            {
                userPoints.erase(it);
                break;
            }
        }
        selectedPoint = 0;
        hoveredPoint = 0;
        update_subdiv(true);
    }
    else if(selectedOrigin)
    {
        reset_horizon();
        update_subdiv(true);
    }
}

uint64_t CurveDetect::get_selected_id()
{
    if(selectedPoint)
        return selectedPoint;
    if(selectedXtick)
        return selectedXtick;
    if(selectedYtick)
        return selectedYtick;
    if(selectedOrigin)
        return selectedOrigin;
    return 0;
}

void CurveDetect::deselect_all()
{
    selectedPoint=0;
    selectedXtick=0;
    selectedYtick=0;
    selectedOrigin=ImageHorizon::NONE;
}

bool CurveDetect::move_selected(Vec2D pos)
{
    auto sel = get_selected();

    if(sel)
        sel->imagePosition = pos;

    return sel!= nullptr;
}

void CurveDetect::check_horizon()
{
    //TODO maybe dont allow to set position too close in the first place
    auto dh=horizon.target.imagePosition-horizon.imagePosition;
    if (std::abs(dh.x) < 2.0f && std::abs(dh.y) < 2.0f)
    {
        horizon.target.imagePosition = horizon.imagePosition + Vec2D(10.0, 0.0);
    }
}

const std::vector<CurveSegment> CurveDetect::get_segments()
{
    return segments;
}

const std::vector<ImagePoint> CurveDetect::get_user_points()
{
    return userPoints;
}

std::vector<ImageTickLine>& CurveDetect::get_xticks()
{
    return xticks;
}

std::vector<ImageTickLine>& CurveDetect::get_yticks()
{
    return yticks;
}

void CurveDetect::set_bin_level(int level)
{
    if(binLevel==level)
        return;
    binLevel=level;
    update_subdiv(true);
}

void CurveDetect::set_invert_image(bool invert)
{
    if(!image->set_inverted(invert))
        return;
    update_subdiv(true);
}


void CurveDetect::set_subdiv_level(int subdiv)
{
    if(subdivLevel==subdiv)
        return;
    subdivLevel=subdiv;
    update_subdiv(true);
}

void CurveDetect::set_curve_thickness(int thick)
{
    if(!image->set_curve_thickness(thick))
        return;
    update_subdiv(true);
}


ImageHorizon CurveDetect::get_horizon() {
    return horizon;
}

void CurveDetect::set_scales(AxisScale xscale, AxisScale yscale)
{
    this->xscale = xscale;
    this->yscale = yscale;
}

PointsBundle::~PointsBundle()
{
//    if(userPointsPixels)
//        delete[](userPointsPixels);
//    if(userPointsReal)
//        delete[](userPointsReal);
//    if(allPointsPixels)
//        delete[](allPointsPixels);
//    if(allPointsReal)
//        delete[](allPointsReal);
}
