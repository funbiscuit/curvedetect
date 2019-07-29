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
    
    SelectedItem = -1;
    HoveredItem = -1;

//    CoordOriginImg = ImVec2(200, 200);
//    CoordOriginTargetX = ImVec2(400, 200);
}


void CurveDetect::UpdateHoveredItemIndex(Vec2D im_pos, int mode)
{
//    ImVec2 WinPos = ImGui::GetWindowPos();
//    ImVec2 MousePos = ImGui::GetMousePos();
    
    HoveredItem = -1;
    double MinDist = SnapDistance;
    double MinDist2 = MinDist*MinDist;
    
    if (mode == ActionMode1_AddPoints)
    {
        for (size_t kp = 0; kp < UserPoints.size(); kp++)
        {
            Vec2D DeltaPos = UserPoints[kp].imagePosition - im_pos;

            double dist2 = DeltaPos.x*DeltaPos.x + DeltaPos.y*DeltaPos.y;
            
            if (dist2 < MinDist2)
            {
                MinDist2 = dist2;
                HoveredItem = kp;
            }
        }
    }
    else if (mode == ActionMode4_AddXTick)
    {
        for (size_t kp = 0; kp < XTicks.size(); kp++)
        {
            //float DeltaPos = std::abs(XTicks[kp].x * im_scale + im_pos.x + WinPos.x - MousePos.x);
            
            Vec2D Direction = horizon.VerticalDirection();
            
            Vec2D Point=XTicks[kp].imagePosition;
            
            double A = Direction.y;
            double B = -Direction.x;
            double C = -A*Point.x - B*Point.y;
            //	float C2 = B*Point.x - A*Point.y;
            
            double DeltaPos = (A*im_pos.x + B*im_pos.y + C);
            DeltaPos = DeltaPos*DeltaPos / (A*A + B*B);
            
            //std::cout << "del: " << DeltaPos << " mx:" << MouseImg.x << "," << MouseImg.y << "\n";
            
            if (DeltaPos < MinDist2)
            {
                MinDist2 = DeltaPos;
                HoveredItem = kp;
            }
        }
    }
    else if (mode == ActionMode5_AddYTick)
    {
        for (size_t kp = 0; kp < YTicks.size(); kp++)
        {
            Vec2D Direction = horizon.HorizontalDirection();

            Vec2D Point=YTicks[kp].imagePosition;
            
            double A = Direction.y;
            double B = -Direction.x;
            double C = -A*Point.x - B*Point.y;


            double DeltaPos = (A*im_pos.x + B*im_pos.y + C);
            DeltaPos = DeltaPos*DeltaPos / (A*A + B*B);
            
            
            if (DeltaPos < MinDist)
            {
                MinDist = DeltaPos;
                HoveredItem = kp;
            }
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
    auto dir1=horizon.targetPosition-horizon.imagePosition;
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
    SortArray(SubdividedPoints);
    
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
    SortArray(SubdividedPoints);
    
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
//    struct sort_class_x
//    {
//        ImVec2 origin;
//        ImVec2 target;
//
//        bool operator() (ImVec2 i, ImVec2 j)
//        {
//            float proj_i = (target.x - origin.x)*(i.x - origin.x) + (target.y - origin.y)*(i.y - origin.y);
//            float proj_j = (target.x - origin.x)*(j.x - origin.x) + (target.y - origin.y)*(j.y - origin.y);
//
//            return proj_i<proj_j;
//            //return (i.x<j.x);
//        }
//    } sort_objectX;
    
    auto origin = horizon.imagePosition;
    auto target = horizon.targetPosition;

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
                
                SnapToCurve(MidPoint);
                SnapToBary(MidPoint);
                SubdividedPoints[n0 + step / 2].imagePosition = MidPoint;
                SubdividedPoints[n0 + step / 2].isSubdivisionPoint = true;
            }
        }
    }

    LastUserPoints = SortedUserPoints;
}

void CurveDetect::SnapToCurve(Vec2D& point)
{
    
    int localX, localY;//to this vars local coords will be written
    
    int hside = (int)SnapDistance;
    
    localX = int(point.x);
    localY = int(point.y);
    
    if (!image->getClosestBlack(localX, localY, hside, BinarizationLevel))
    {
        //std::cout << "Bad point!\n";
        return;
    }
    
    if (localX != int(point.x))
    {
        point.x = float(localX);
    }
    if (localY != int(point.y))
    {
        point.y = float(localY);
    }
    //std::cout << "loc snap: (" << snapX << "," << snapY << ")\n";
}

void CurveDetect::SnapToBary(Vec2D& point)
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
        return;
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
    }
    
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

    auto dir1=horizon.targetPosition-horizon.imagePosition;
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
    
    XTicks.clear();
    YTicks.clear();

    horizon.targetPosition=horizon.imagePosition + Vec2D(100.0, 0.0);
    
    
    if (image)
    {
        horizon.imagePosition.x=image->get_width()*0.1;
        horizon.imagePosition.y=image->get_height()*0.5;
        horizon.targetPosition.x=image->get_width()*0.9;
        horizon.targetPosition.y=image->get_height()*0.5;
    }
    
}

void CurveDetect::AddPoint(Vec2D pos)
{
    SnapToCurve(pos);
    SnapToBary(pos);
    UserPoints.emplace_back(pos);
    SortPoints();
    UpdateSubdivision();
    SelectedItem = UserPoints.size() - 1;
}

int CurveDetect::GetHoveredPoint()
{
    if(HoveredItem >= 0 && HoveredItem < int(UserPoints.size()))
        return HoveredItem;
    else
        return -1;
}

void CurveDetect::DeleteHoveredPoint()
{
    UserPoints.erase(UserPoints.begin() + HoveredItem);
    HoveredItem = -1;
    SelectedItem = -1;
    SortPoints();
    UpdateSubdivision();
}

void CurveDetect::SelectHovered()
{
    SelectedItem = HoveredItem;
}

void CurveDetect::SetOrigin(Vec2D pos, bool snap)
{
    if (snap)
    {
        SnapToCurve(pos);
        SnapToBary(pos);
    }
    horizon.imagePosition = pos;
    
    SortPoints();
    UpdateSubdivision();
}

void CurveDetect::SetTarget(Vec2D pos, bool snap)
{
    if (snap)
    {
        SnapToCurve(pos);
        SnapToBary(pos);
    }

    horizon.targetPosition = pos;
    SortPoints();
    UpdateSubdivision();
}

bool CurveDetect::AddXTick(Vec2D pos)
{
    if(XTicks.size()==2)
        return false;
    
    XTicks.emplace_back(pos);
    SelectedItem = XTicks.size() - 1;
    
    return true;
}

int CurveDetect::GetHoveredXTick()
{
    if(HoveredItem >= 0 && HoveredItem < int(XTicks.size()))
        return HoveredItem;
    else
        return -1;
}

void CurveDetect::DeleteHoveredXTick()
{
    XTicks.erase(XTicks.begin() + HoveredItem);
    HoveredItem = -1;
    SelectedItem = -1;
}

bool CurveDetect::AddYTick(Vec2D pos)
{
    if(YTicks.size()==2)
        return false;

    YTicks.emplace_back(pos);
    SelectedItem = YTicks.size() - 1;
    
    return true;
}

int CurveDetect::GetHoveredYTick()
{
    if(HoveredItem >= 0 && HoveredItem < int(YTicks.size()))
        return HoveredItem;
    else
        return -1;
}

void CurveDetect::DeleteHoveredYTick()
{
    YTicks.erase(YTicks.begin() + HoveredItem);
    HoveredItem = -1;
    SelectedItem = -1;
}

int CurveDetect::GetSelected()
{
    return SelectedItem;
}

int CurveDetect::GetHovered()
{
    return HoveredItem;
}

void CurveDetect::MoveSelectedPoint(Vec2D pos, bool snap)
{
    if (snap)
    {
        SnapToCurve(pos);
        SnapToBary(pos);
    }
    UserPoints[SelectedItem].imagePosition = pos;
    SortPoints();
    UpdateSubdivision();
}

void CurveDetect::MoveSelectedXTick(Vec2D pos, bool snap)
{
    if (snap)
    {
        SnapToCurve(pos);
        SnapToBary(pos);
    }
    XTicks[SelectedItem].imagePosition = pos;
}

void CurveDetect::MoveSelectedYTick(Vec2D pos, bool snap)
{
    if (snap)
    {
        SnapToCurve(pos);
        SnapToBary(pos);
    }
    YTicks[SelectedItem].imagePosition = pos;
}

void CurveDetect::CheckTarget()
{
    //TODO maybe dont allow to set position too close in the first place
    auto dh=horizon.targetPosition-horizon.imagePosition;
    if (std::abs(dh.x) < 2.0f && std::abs(dh.y) < 2.0f)
    {
        horizon.targetPosition = horizon.imagePosition + Vec2D(10.0, 0.0);
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

