#include "curve_detect.h"


#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>

#include <algorithm>
#include <cmath>
#include "mat_file_writer.h"
#include "main_app.h"



CurveDetect::CurveDetect(std::shared_ptr<Image> image) : horizon(Vec2D(100,100))
{
    this->image=image;
    
    BinarizationLevel = 127;
    
    SnapDistance=30.f;
    
    SubdivideIterations = 3;
}

uint64_t CurveDetect::GetHoveredId(int selectionFilter)
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

void CurveDetect::UpdateHoveredItem(Vec2D imagePos)
{
//    auto prevHovered=GetHoveredId(ImageElement::ALL);

    UpdateHoveredPoint(imagePos);
    UpdateHoveredTickX(imagePos);
    UpdateHoveredTickY(imagePos);
    UpdateHoveredHorizon(imagePos);

//    auto newHovered = GetHoveredId(ImageElement::ALL);
}

void CurveDetect::UpdateHoveredPoint(Vec2D imagePos)
{
    hoveredPoint = 0;
    double minDist = hoverZone * hoverZone;

    for (auto& point : UserPoints)
    {
        double dist = (point.imagePosition - imagePos).norm2();

        if(dist<minDist)
        {
            minDist = dist;
            hoveredPoint = point.id;
        }
    }
}

void CurveDetect::UpdateHoveredHorizon(Vec2D imagePos)
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

void CurveDetect::UpdateHoveredTickX(Vec2D imagePos)
{
    hoveredXtick = 0;
    double minDist = hoverZone;
    Vec2D tickDir = horizon.VerticalDirection();

    for (auto& tick : XTicks)
    {
        double dist = tick.DistTo(imagePos, tickDir);

        if(dist<minDist)
        {
            minDist = dist;
            hoveredXtick = tick.id;
        }
    }
}

void CurveDetect::UpdateHoveredTickY(Vec2D imagePos)
{
    hoveredYtick = 0;
    double minDist = hoverZone;
    Vec2D tickDir = horizon.HorizontalDirection();

    for (auto& tick : YTicks)
    {
        double dist = tick.DistTo(imagePos, tickDir);

        if(dist<minDist)
        {
            minDist = dist;
            hoveredYtick = tick.id;
        }
    }
}

bool CurveDetect::IsReadyForExport(int& out_Result)
{
    
    out_Result = ExportReadyStatus::ExportReadyStatus_Ready;
    
    if (UserPoints.size() < 2)
    {
        out_Result |= ExportReadyStatus::ExportReadyStatus_NoPoints;
    }
    auto dir1=horizon.target.imagePosition-horizon.imagePosition;
    double norm = std::sqrt(dir1.x*dir1.x + dir1.y*dir1.y);
    if (XTicks.size() < 2)
    {
        out_Result |= ExportReadyStatus::ExportReadyStatus_NoXTicks;
    }
    else
    {
        auto dxtick=XTicks[0].imagePosition-XTicks[1].imagePosition;

        //for deriving a and b
        double det1 = (dxtick.x*dir1.x + dxtick.y*dir1.y)/norm;
        
        //std::cout << "det1: "<< det1 << "\n";
        
        //float dist=det1/
        
        if (std::abs(det1) < MinTickPixelDistance)
        {
            out_Result |= ExportReadyStatus::ExportReadyStatus_XTicksSimilarPositions;
        }
        else if (std::abs(XTicks[0].tickValue - XTicks[1].tickValue) < MinTickRealDistance)
        {
            out_Result |= ExportReadyStatus::ExportReadyStatus_XTicksSimilarValues;
        }
    }
    
    if (YTicks.size() < 2)
    {
        out_Result |= ExportReadyStatus::ExportReadyStatus_NoYTicks;
    }
    else
    {
        auto dytick=YTicks[0].imagePosition-YTicks[1].imagePosition;

        //for deriving c and d
        double det2 = (-dytick.x*dir1.y + dytick.y*dir1.x)/norm;
        
        //std::cout << "det2: " << det2 << "\n";
        
        if (std::abs(det2)<MinTickPixelDistance)
        {
            out_Result |= ExportReadyStatus::ExportReadyStatus_YTicksSimilarPositions;
        }
        else if (std::abs(YTicks[0].tickValue - YTicks[1].tickValue)<MinTickRealDistance)
        {
            out_Result |= ExportReadyStatus::ExportReadyStatus_YTicksSimilarValues;
        }
    }
    
    return out_Result == ExportReadyStatus_Ready;
}

void CurveDetect::ExportToClipboard(std::string columnSeparator,
        std::string lineEnding, char decimalSeparator)
{
    
    int out_Result;
    if (!IsReadyForExport(out_Result))
        return;

    UpdateSubdivision();
    
    Vec2D RealPoint;
    std::stringstream sstr;
    //std::stringstream sstr;
    
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
    
    for (size_t kp = 0; kp < SubdividedPoints.size(); kp++)
    {
        RealPoint = ConvertImageToReal(SubdividedPoints[kp].imagePosition);
        
        numToStr(RealPoint.x, decimalSeparator, nums);
        
        sstr << nums;
        sstr << tempColumnSeparator;
    
        numToStr(RealPoint.y, decimalSeparator, nums);
        sstr<< nums;
        sstr << tempLineEnding;
        //sstr << newLineSeq;
    }
    
    //TODO should not be here
    MainApp::getInstance().copy_to_clipboard(sstr.str().c_str());
}

void CurveDetect::numToStr(double num, char decimalSeparator, std::string& out_String)
{
    out_String = std::to_string(num);
    
    if (decimalSeparator == '.')
        std::replace(out_String.begin(), out_String.end(), ',', '.');
    else if (decimalSeparator == ',')
        std::replace(out_String.begin(), out_String.end(), '.', ',');
    
    //std::cout << out_String << "\n";
}

void CurveDetect::ExportPoints(const char* path, bool asText)
{
    UpdateSubdivision();
    
    Vec2D RealPoint;
    
    std::vector<Vec2D> RealUserPoints(UserPoints.size());
    std::vector<Vec2D> RealSubdividedPoints(SubdividedPoints.size());
    
    
    std::ofstream ofs;
    
    if(asText)
        ofs.open(path);// , std::ofstream::out | std::ofstream::app);
    
    
    
    
    for (size_t kp = 0; kp < SubdividedPoints.size(); kp++)
    {
        RealPoint = ConvertImageToReal(SubdividedPoints[kp].imagePosition);
        RealSubdividedPoints[kp] = RealPoint;
        
        if (asText)
        {
            ofs << RealPoint.x << "\t" << RealPoint.y << "\n";
        }
        
    }
    
    if (asText)
    {
        ofs.close();
        return;
    }
    
    for (size_t kp = 0; kp < UserPoints.size(); kp++)
    {
        RealPoint = ConvertImageToReal(UserPoints[kp].imagePosition);
        RealUserPoints[kp] = RealPoint;
    }
    
    
    FILE* fp=fopen(path, "wb");
    
    if (fp != nullptr)
    {
        writeHeader(fp);
        writeMatrixToMatFile(fp, "UserPointsPixels", &(UserPoints[0].imagePosition.x), UserPoints.size(), 2);
        writeMatrixToMatFile(fp, "SubdividedPointsPixels", &(SubdividedPoints[0].imagePosition.x), SubdividedPoints.size(), 2);
        writeMatrixToMatFile(fp, "UserPointsReal", &(RealUserPoints[0].x), RealUserPoints.size(), 2);
        writeMatrixToMatFile(fp, "SubdividedPointsReal", &(RealSubdividedPoints[0].x), RealSubdividedPoints.size(), 2);
    }
    
    fclose(fp);
}


void CurveDetect::SortPoints()
{
    SortArray(UserPoints);
}


void CurveDetect::SortArray(std::vector<ImagePoint>& Array)
{
    auto origin = horizon.imagePosition;
    auto target = horizon.target.imagePosition;

    std::sort(Array.begin(), Array.end(), [&origin, &target](const ImagePoint &lhs, const ImagePoint &rhs)
    {
        double proj_i = (target.x - origin.x)*(lhs.imagePosition.x - origin.x) + (target.y - origin.y)*(lhs.imagePosition.y - origin.y);
        double proj_j = (target.x - origin.x)*(rhs.imagePosition.x - origin.x) + (target.y - origin.y)*(rhs.imagePosition.y - origin.y);

        return proj_i<proj_j;
    });
}

bool CurveDetect::IsArraySorted(std::vector<ImagePoint>& Array)
{
    auto origin = horizon.imagePosition;
    auto target = horizon.target.imagePosition;

    return std::is_sorted(Array.begin(), Array.end(), [&origin, &target](const ImagePoint &lhs, const ImagePoint &rhs)
    {
        double proj_i = (target.x - origin.x)*(lhs.imagePosition.x - origin.x) + (target.y - origin.y)*(lhs.imagePosition.y - origin.y);
        double proj_j = (target.x - origin.x)*(rhs.imagePosition.x - origin.x) + (target.y - origin.y)*(rhs.imagePosition.y - origin.y);

        return proj_i<proj_j;
    });
}

void CurveDetect::UpdateSubdivision()
{
    //when fast update is true then only intervals where end positions change will be updated
    //everything else should be the same so we don't bother updating them
    bool isSorted = IsArraySorted(UserPoints);
    bool fastUpdate = subdivThreshold == BinarizationLevel && isSorted;
    subdivThreshold = BinarizationLevel;
    int userPointsCount = UserPoints.size();
    
    if (userPointsCount < 2)
    {
        SubdividedPoints.clear();
        return;
    }
    if(!isSorted)
        SortPoints();
    
    
    //extra points for each two user points
    int extraPoints = (1 << SubdivideIterations) - 1;//=(2^S)-1

    //number of points we need to move from one border to mid point (excluding)
    int extraPointsHalf = (1 << (SubdivideIterations-1));

    int allPointsCount = (extraPoints + 1)*(userPointsCount - 1) + 1;

    if (SubdividedPoints.size() != allPointsCount)
    {
        SubdividedPoints.clear();
        SubdividedPoints.resize(allPointsCount);
        for(int i=0; i < userPointsCount; ++i)
            SubdividedPoints[i*(extraPoints+1)].imagePosition = UserPoints[i].imagePosition;
        fastUpdate=false;
    }

    bool forceSubdiv = false;

    int step =extraPoints + 1;
    for (int i = 0; i < userPointsCount-1; ++i)
    {
        //check if we need to subdivide this segment

        int left = i*step;
        int right = left + step;

        Vec2D leftPos = SubdividedPoints[left].imagePosition;
        Vec2D rightPos = SubdividedPoints[right].imagePosition;
        const Vec2D& leftPosNew = UserPoints[i].imagePosition;
        const Vec2D& rightPosNew = UserPoints[i+1].imagePosition;


        //check that both ends didnt move (if it is allowed to check this)
        if(!forceSubdiv && fastUpdate && leftPos==leftPosNew && rightPos == rightPosNew)
            continue;
        else
        {
            forceSubdiv = !(rightPos == rightPosNew);
            SubdividedPoints[left].imagePosition = leftPosNew;
            SubdividedPoints[right].imagePosition = rightPosNew;
        }

        for(int j=1; j<= extraPointsHalf; ++j)
        {
            leftPos = SubdividedPoints[left].imagePosition;
            rightPos = SubdividedPoints[right].imagePosition;

            auto& nextLeft = SubdividedPoints[left+1];

            nextLeft.imagePosition = leftPos + (rightPos-leftPos)/double(right-left);
            nextLeft.isSnapped = Snap(nextLeft.imagePosition);
            nextLeft.isSubdivisionPoint = true;

            if(j!=extraPointsHalf)
            {
                auto& nextRight = SubdividedPoints[right-1];
                nextRight.imagePosition = rightPos - (rightPos-leftPos)/double(right-left);
                nextRight.isSnapped = Snap(nextRight.imagePosition);
                nextRight.isSubdivisionPoint = true;
            }

            ++left;
            --right;
        }
    }
}

bool CurveDetect::Snap(Vec2D& pos)
{
    return image ? image->SnapToCurve(pos, BinarizationLevel, (int)SnapDistance) &&
                    image->SnapToBary(pos, BinarizationLevel) : false;
}

Vec2D CurveDetect::ConvertImageToReal(const Vec2D& point)
{
    Vec2D RealPoint;
    
    //this function should be called after check that
    //we can really calculate real points
    //so all x and y ticks are defined
    
    Vec2D Scale, Offset;

    auto dir1=horizon.target.imagePosition-horizon.imagePosition;
    auto dxtick=XTicks[0].imagePosition-XTicks[1].imagePosition;
    auto dytick=YTicks[0].imagePosition-YTicks[1].imagePosition;
    
    Scale.x = (XTicks[0].tickValue - XTicks[1].tickValue) / dxtick.x;
    Scale.y = (YTicks[0].tickValue - YTicks[1].tickValue) / dytick.y;
    
    
    Offset.x = XTicks[1].tickValue - XTicks[1].X()*Scale.x;
    Offset.y = YTicks[1].tickValue - YTicks[1].Y()*Scale.y;
    
    //XTicks[1].z + (ImagePoint.x- XTicks[1].x)*Scale.x
    
    RealPoint.x = Offset.x + point.x*Scale.x;
    RealPoint.y = Offset.y + point.y*Scale.y;
    
    //for deriving a and b
    double det1 = dxtick.x*dir1.x + dxtick.y*dir1.y;

    //for deriving c and d
    double det2 = -dytick.x*dir1.y + dytick.y*dir1.x;

    double a, b, c, d, e, f;
    
    a =  (XTicks[0].tickValue - XTicks[1].tickValue)*dir1.x / det1;
    b =  (XTicks[0].tickValue - XTicks[1].tickValue)*dir1.y / det1;
    
    c = -(YTicks[0].tickValue - YTicks[1].tickValue)*dir1.y / det2;
    d =  (YTicks[0].tickValue - YTicks[1].tickValue)*dir1.x / det2;
    
    e = XTicks[0].tickValue - a*XTicks[0].X() - b*XTicks[0].Y();
    f = YTicks[0].tickValue - c*YTicks[0].X() - d*YTicks[0].Y();
    
    RealPoint.x = a*point.x + b*point.y + e;
    RealPoint.y = c*point.x + d*point.y + f;
    
    
    return RealPoint;
}


void CurveDetect::ResetAll()
{
    UserPoints.clear();
    SubdividedPoints.clear();
    subdivThreshold = -1;
    
    XTicks.resize(2);
    YTicks.resize(2);

    XTicks[0].setValueStr("0");
    XTicks[1].setValueStr("1");
    YTicks[0].setValueStr("0");
    YTicks[1].setValueStr("1");
    
    if (image)
    {
        XTicks[0].imagePosition.x = image->get_width()*0.2;
        XTicks[0].imagePosition.y = image->get_height()*0.5;
        XTicks[1].imagePosition.x = image->get_width()*0.8;
        XTicks[1].imagePosition.y = image->get_height()*0.5;
    
        YTicks[0].imagePosition.x = image->get_width()*0.5;
        YTicks[0].imagePosition.y = image->get_height()*0.8;
        YTicks[1].imagePosition.x = image->get_width()*0.5;
        YTicks[1].imagePosition.y = image->get_height()*0.2;
    }

    DeselectAll();

    ResetHorizon();
    
}
void CurveDetect::ResetHorizon()
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

void CurveDetect::SnapSelected()
{
    auto sel = GetSelected();

    if(sel && image)
        sel->isSnapped = Snap(sel->imagePosition);
}

void CurveDetect::BackupSelectedTick()
{
    auto sel = (ImageTickLine*) GetSelected();
    if(sel != nullptr)
        sel->MakeBackup();
}

void CurveDetect::AddPoint(Vec2D pos)
{
    UserPoints.emplace_back(pos);
    selectedPoint = UserPoints.back().id;
    SnapSelected();
    UpdateSubdivision();
}

bool CurveDetect::SelectHovered(int selectionFilter)
{
    if( (selectionFilter & ImageElement::POINT) && hoveredPoint )
    {
        DeselectAll();
        selectedPoint = hoveredPoint;
        return true;
    }
    if( (selectionFilter & ImageElement::X_TICK) && hoveredXtick )
    {
        DeselectAll();
        selectedXtick = hoveredXtick;
        return true;
    }
    if( (selectionFilter & ImageElement::Y_TICK) && hoveredYtick )
    {
        DeselectAll();
        selectedYtick = hoveredYtick;
        return true;
    }
    if( (selectionFilter & ImageElement::HORIZON) && hoveredOrigin )
    {
        DeselectAll();
        selectedOrigin = hoveredOrigin;
        return true;
    }

    return false;
}

ImageElement* CurveDetect::GetSelected()
{
    if(selectedPoint)
    {
        for (auto &point : UserPoints)
            if (point.id == selectedPoint)
                return &point;
    }
    else if(selectedXtick)
    {
        for (auto &tick : XTicks)
            if(tick.id == selectedXtick)
                return &tick;
    }
    else if(selectedYtick)
    {
        for (auto &tick : YTicks)
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

void CurveDetect::DeleteSelected()
{
    if(selectedPoint)
    {
        for(auto it=UserPoints.begin();it!=UserPoints.end();++it)
        {
            if(it->id == selectedPoint)
            {
                UserPoints.erase(it);
                break;
            }
        }
        selectedPoint = 0;
        hoveredPoint = 0;
        UpdateSubdivision();
    }
    else if(selectedOrigin)
    {
        ResetHorizon();
        UpdateSubdivision();
    }
}

uint64_t CurveDetect::GetSelectedId()
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

void CurveDetect::DeselectAll()
{
    selectedPoint=0;
    selectedXtick=0;
    selectedYtick=0;
    selectedOrigin=ImageHorizon::NONE;
}

bool CurveDetect::MoveSelected(Vec2D pos)
{
    auto sel = GetSelected();

    if(sel)
        sel->imagePosition = pos;

    return sel!= nullptr;
}

void CurveDetect::CheckTarget()
{
    //TODO maybe dont allow to set position too close in the first place
    auto dh=horizon.target.imagePosition-horizon.imagePosition;
    if (std::abs(dh.x) < 2.0f && std::abs(dh.y) < 2.0f)
    {
        horizon.target.imagePosition = horizon.imagePosition + Vec2D(10.0, 0.0);
    }
}

const std::vector<ImagePoint> CurveDetect::GetAllPoints()
{
    return SubdividedPoints;
}

const std::vector<ImagePoint> CurveDetect::GetUserPoints()
{
    return UserPoints;
}

std::vector<ImageTickLine>& CurveDetect::GetXTicks()
{
    return XTicks;
}

std::vector<ImageTickLine>& CurveDetect::GetYTicks()
{
    return YTicks;
}

void CurveDetect::SetBinarizationLevel(int level)
{
    if(BinarizationLevel==level)
        return;
    BinarizationLevel=level;
    UpdateSubdivision();
}


void CurveDetect::SetSubdivIterations(int subdiv)
{
    if(SubdivideIterations==subdiv)
        return;
    SubdivideIterations=subdiv;
    UpdateSubdivision();
}

ImageHorizon CurveDetect::GetHorizon() {
    return horizon;
}

