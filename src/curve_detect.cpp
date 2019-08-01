#include "curve_detect.h"


#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>

#include <algorithm>
#include "mat_file_writer.h"

//TODO should not be here
#include "main_window.h"

#include "imgui_helpers.h"

#include <curve_detect.h>
#include <main_app.h>



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
    
    if (SortedUserPoints.size() < 2)
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
    
    SortPoints();
    UpdateSubdivision(true);
//    SortArray(SubdividedPoints);
    
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
    SortPoints();
    UpdateSubdivision(true);
//    SortArray(SubdividedPoints);
    
    Vec2D RealPoint;
    
    std::vector<Vec2D> RealUserPoints(SortedUserPoints.size());
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
    
    for (size_t kp = 0; kp < SortedUserPoints.size(); kp++)
    {
        RealPoint = ConvertImageToReal(SortedUserPoints[kp].imagePosition);
        RealUserPoints[kp] = RealPoint;
    }
    
    
    FILE* fp=fopen(path, "wb");
    
    if (fp != nullptr)
    {
        writeHeader(fp);
        writeMatrixToMatFile(fp, "UserPointsPixels", &(SortedUserPoints[0].imagePosition.x), SortedUserPoints.size(), 2);
        writeMatrixToMatFile(fp, "SubdividedPointsPixels", &(SubdividedPoints[0].imagePosition.x), SubdividedPoints.size(), 2);
        writeMatrixToMatFile(fp, "UserPointsReal", &(RealUserPoints[0].x), RealUserPoints.size(), 2);
        writeMatrixToMatFile(fp, "SubdividedPointsReal", &(RealSubdividedPoints[0].x), RealSubdividedPoints.size(), 2);
    }
    
    fclose(fp);
}


void CurveDetect::SortPoints()
{
    SortedUserPoints = UserPoints;
    
    SortArray(SortedUserPoints);
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

//    sort(Array.begin(), Array.end(), sort_objectX);
}

void CurveDetect::UpdateSubdivision(bool bUpdateAll)
{
    int PointsNum = SortedUserPoints.size();
    
    if (PointsNum < 2)
    {
        SubdividedPoints.clear();
        return;
    }
    
    
    //extra points for each two user points
    //int ExtraPoints = std::pow(2, SubdivideIterations) - 1;
    int ExtraPoints = (1 << SubdivideIterations) - 1;//=(2^S)-1
    
    //std::cout << "ep: " << ExtraPoints << " new: " << (1 << SubdivideIterations)-1 << "\n";
    
    int SubPointNum = (ExtraPoints + 1)*(PointsNum - 1) + 1;
    
    
    static std::vector<ImagePoint> LastUserPoints;
    
    
    if (SubdividedPoints.size() != SubPointNum)
        SubdividedPoints.resize(SubPointNum);
    
    
    
    //copy existing point to new array
    for (size_t k = 0; k < SortedUserPoints.size(); k++)
    {
        SubdividedPoints[k*(ExtraPoints+1)] = SortedUserPoints[k];
    }
    
    
    
    auto PrevPoints = SubdividedPoints;
    for (size_t k = 0; k < SortedUserPoints.size()-1; k++)
    {
        if (SortedUserPoints.size() == LastUserPoints.size())
        {
            if (!bUpdateAll
                && SortedUserPoints[k].X() == LastUserPoints[k].X()
                && SortedUserPoints[k].Y() == LastUserPoints[k].Y()
                && SortedUserPoints[k+1].X() == LastUserPoints[k+1].X()
                && SortedUserPoints[k+1].Y() == LastUserPoints[k+1].Y())
            {
                continue;
            }
        }
        
        Vec2D MidPoint = SortedUserPoints[k].imagePosition;
        for (int k_it = 0; k_it < SubdivideIterations; k_it++)
        {
            //k*(ExtraPoints + 1) - left, (k+1)*(ExtraPoints+1) - right
            
            int step = (1 << (SubdivideIterations-k_it));// for 0 it
            
            for (int k_p = 0; k_p < ExtraPoints + 1; k_p+=step)
            {
                int n0 = k*(ExtraPoints + 1) + k_p;
                
                MidPoint = (SubdividedPoints[n0].imagePosition + SubdividedPoints[n0 + step].imagePosition) * 0.5f;

                SubdividedPoints[n0 + step / 2].isSnapped = SnapToCurve(MidPoint) && SnapToBary(MidPoint);
                SubdividedPoints[n0 + step / 2].imagePosition = MidPoint;
                SubdividedPoints[n0 + step / 2].isSubdivisionPoint = true;
            }
        }
    }

    LastUserPoints = SortedUserPoints;
}

bool CurveDetect::SnapToCurve(Vec2D& point)
{
    
    int localX, localY;//to this vars local coords will be written
    
    int hside = (int)SnapDistance;
    
    localX = int(point.x);
    localY = int(point.y);
    
    if (!image->getClosestBlack(localX, localY, hside, BinarizationLevel))
    {
        //std::cout << "Bad point!\n";
        return false;
    }
    
    if (localX != int(point.x))
    {
        point.x = float(localX);
    }
    if (localY != int(point.y))
    {
        point.y = float(localY);
    }
    return true;
    //std::cout << "loc snap: (" << snapX << "," << snapY << ")\n";
}

bool CurveDetect::SnapToBary(Vec2D& point)
{
    int localX, localY;//to this vars local coords will be written
    
    int baryX, baryY; //local coords of snapped point
    
    int hside = 3;// ZoomPixelHSide;
    
    localX = int(point.x);
    localY = int(point.y);
    
    MatrixXi PointRegion;
    
    if (!image->getNearbyPoints(localX, localY, hside, PointRegion))
    {
        std::cout << "Bad point!\n";
        return false;
    }
    
    //by default just leave point where it was
    baryX = localX;
    baryY = localY;
    
    
    
    int baryMass = 0;
    
    Vec2D BaryOffset;
    
    for (int kx = 0; kx <= 2 * hside; kx++)
    {
        for (int ky = 0; ky <= 2 * hside; ky++)
        {
            if (PointRegion(ky, kx) < BinarizationLevel)
            {
                baryMass += BinarizationLevel - PointRegion(ky, kx);
                
                BaryOffset += Vec2D(double(kx - localX), double(ky - localY))*double(BinarizationLevel - PointRegion(ky, kx));
                
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
    
    //ImagePoint.x += (baryX - localX);
    //ImagePoint.y += (baryY - localY);
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
    SortedUserPoints.clear();
    SubdividedPoints.clear();
    
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
        sel->isSnapped = SnapToCurve(sel->imagePosition) && SnapToBary(sel->imagePosition);
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
    SortPoints();
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
        SortPoints();
        UpdateSubdivision(true);
    }
    else if(selectedOrigin)
    {
        ResetHorizon();
        SortPoints();
        UpdateSubdivision(true);
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
    UpdateSubdivision(true);
}


void CurveDetect::SetSubdivIterations(int subdiv)
{
    if(SubdivideIterations==subdiv)
        return;
    SubdivideIterations=subdiv;
    UpdateSubdivision(true);
}

ImageHorizon CurveDetect::GetHorizon() {
    return horizon;
}

