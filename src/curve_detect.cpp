#include "curve_detect.h"


#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>

#include <cstdlib>
#include <algorithm>
#include "mat_file_writer.h"

//TODO should not be here
#include "main_window.h"

#include "imgui_helpers.h"

#include <Eigen/Dense>
#include <filesystem>
#include <curve_detect.h>
#include <main_app.h>


using Eigen::MatrixXd;
using Eigen::MatrixXi;
using Eigen::Vector2d;




CurveDetect::CurveDetect(std::shared_ptr<Image> image)
{
    this->image=image;
    
    BinarizationLevel = 127;
    
    SnapDistance=30.f;
    
    SubdivideIterations = 3;
    
    SelectedItem = -1;
    HoveredItem = -1;
    
    CoordOriginImg = ImVec2(200, 200);
    CoordOriginTargetX = ImVec2(400, 200);
}


void CurveDetect::UpdateHoveredItemIndex(ImVec2 im_pos, int mode)
{
//    ImVec2 WinPos = ImGui::GetWindowPos();
//    ImVec2 MousePos = ImGui::GetMousePos();
    
    HoveredItem = -1;
    float MinDist = SnapDistance;
    float MinDist2 = MinDist*MinDist;
    
    if (mode == ActionMode1_AddPoints)
    {
        for (size_t kp = 0; kp < UserPoints.size(); kp++)
        {
            ImVec2 DeltaPos = UserPoints[kp] - im_pos;
            
            float dist2 = DeltaPos.x*DeltaPos.x + DeltaPos.y*DeltaPos.y;
            
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
            
            ImVec2 Direction;
            Direction.x = CoordOriginTargetX.y - CoordOriginImg.y;
            Direction.y = -(CoordOriginTargetX.x - CoordOriginImg.x);
            
            ImVec2 Point;
            Point.x = XTicks[kp].x;
            Point.y = XTicks[kp].y;
            
            float A = Direction.y;
            float B = -Direction.x;
            float C = -A*Point.x - B*Point.y;
            //	float C2 = B*Point.x - A*Point.y;
            
            
            ImVec2 MouseImg = im_pos;
            
            float DeltaPos = (A*MouseImg.x + B*MouseImg.y + C);
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
            ImVec2 Direction;
            Direction.x = CoordOriginTargetX.x - CoordOriginImg.x;
            Direction.y = CoordOriginTargetX.y - CoordOriginImg.y;
            
            
            
            ImVec2 Point;
            Point.x = YTicks[kp].x;
            Point.y = YTicks[kp].y;
            
            float A = Direction.y;
            float B = -Direction.x;
            float C = -A*Point.x - B*Point.y;
    
    
            ImVec2 MouseImg = im_pos;
            
            float DeltaPos = (A*MouseImg.x + B*MouseImg.y + C);
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
    if (XTicks.size() < 2)
    {
        out_Result |= ExportReadyStatus::ExportReadyStatus_NoXTicks;
    }
    else
    {
        float norm = std::sqrt((CoordOriginTargetX.x - CoordOriginImg.x)*(CoordOriginTargetX.x - CoordOriginImg.x) +
                               (CoordOriginTargetX.y - CoordOriginImg.y)*(CoordOriginTargetX.y - CoordOriginImg.y));
        
        //for deriving a and b
        float det1 = ((XTicks[0].x - XTicks[1].x)*(CoordOriginTargetX.x - CoordOriginImg.x)
                      + (XTicks[0].y - XTicks[1].y)*(CoordOriginTargetX.y - CoordOriginImg.y))/norm;
        
        //std::cout << "det1: "<< det1 << "\n";
        
        //float dist=det1/
        
        if (std::abs(det1) < MinTickPixelDistance)
        {
            out_Result |= ExportReadyStatus::ExportReadyStatus_XTicksSimilarPositions;
        }
        else if (std::abs(XTicks[0].z - XTicks[1].z) < MinTickRealDistance)
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
        float norm = std::sqrt((CoordOriginTargetX.x - CoordOriginImg.x)*(CoordOriginTargetX.x - CoordOriginImg.x) +
                               (CoordOriginTargetX.y - CoordOriginImg.y)*(CoordOriginTargetX.y - CoordOriginImg.y));
        
        //for deriving c and d
        float det2 = ((YTicks[0].x - YTicks[1].x)*(CoordOriginImg.y - CoordOriginTargetX.y)
                      - (YTicks[0].y - YTicks[1].y)*(CoordOriginImg.x - CoordOriginTargetX.x))/norm;
        
        //std::cout << "det2: " << det2 << "\n";
        
        if (std::abs(det2)<MinTickPixelDistance)
        {
            out_Result |= ExportReadyStatus::ExportReadyStatus_YTicksSimilarPositions;
        }
        else if (std::abs(YTicks[0].z - YTicks[1].z)<MinTickRealDistance)
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
    
    ImVec2 RealPoint;
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
        RealPoint = ConvertImageToReal(SubdividedPoints[kp]);
        
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

void CurveDetect::numToStr(float num, char decimalSeparator, std::string& out_String)
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
    
    ImVec2 RealPoint;
    
    std::vector<ImVec2> RealUserPoints = SortedUserPoints;
    std::vector<ImVec2> RealSubdividedPoints = SubdividedPoints;
    
    
    std::ofstream ofs;
    
    if(asText)
        ofs.open(path);// , std::ofstream::out | std::ofstream::app);
    
    
    
    
    for (size_t kp = 0; kp < SubdividedPoints.size(); kp++)
    {
        RealPoint = ConvertImageToReal(SubdividedPoints[kp]);
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
        RealPoint = ConvertImageToReal(SortedUserPoints[kp]);
        RealUserPoints[kp] = RealPoint;
    }
    
    
    FILE* fp=fopen(path, "wb");
    
    if (fp != nullptr)
    {
        writeHeader(fp);
        writeMatrixToMatFile(fp, "UserPointsPixels", &(SortedUserPoints[0].x), SortedUserPoints.size(), 2);
        writeMatrixToMatFile(fp, "SubdividedPointsPixels", &(SubdividedPoints[0].x), SubdividedPoints.size(), 2);
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


void CurveDetect::SortArray(std::vector<ImVec2>& Array)
{
    struct sort_class_x
    {
        ImVec2 origin;
        ImVec2 target;
        
        bool operator() (ImVec2 i, ImVec2 j)
        {
            float proj_i = (target.x - origin.x)*(i.x - origin.x) + (target.y - origin.y)*(i.y - origin.y);
            float proj_j = (target.x - origin.x)*(j.x - origin.x) + (target.y - origin.y)*(j.y - origin.y);
            
            return proj_i<proj_j;
            //return (i.x<j.x);
        }
    } sort_objectX;
    
    sort_objectX.origin = CoordOriginImg;
    sort_objectX.target = CoordOriginTargetX;
    
    sort(Array.begin(), Array.end(), sort_objectX);
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
    
    
    static std::vector<ImVec2> LastUserPoints;
    
    
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
                && SortedUserPoints[k].x == LastUserPoints[k].x
                && SortedUserPoints[k].y == LastUserPoints[k].y
                && SortedUserPoints[k+1].x == LastUserPoints[k+1].x
                && SortedUserPoints[k+1].y == LastUserPoints[k+1].y)
            {
                continue;
            }
        }
        
        ImVec2 MidPoint = SortedUserPoints[k];
        for (int k_it = 0; k_it < SubdivideIterations; k_it++)
        {
            //k*(ExtraPoints + 1) - left, (k+1)*(ExtraPoints+1) - right
            
            int step = (1 << (SubdivideIterations-k_it));// for 0 it
            
            for (int k_p = 0; k_p < ExtraPoints + 1; k_p+=step)
            {
                int n0 = k*(ExtraPoints + 1) + k_p;
                
                MidPoint = (SubdividedPoints[n0] + SubdividedPoints[n0 + step]) * 0.5f;
                
                SnapToCurve(MidPoint);
                SnapToBary(MidPoint);
                SubdividedPoints[n0 + step / 2] = MidPoint;
            }
        }
        
        
        //if(!bSmoothPoints)
        continue;
        
        //std::cout << "---------\n";
        
        PrevPoints = SubdividedPoints;
        
        for (int k_p = 0; k_p < ExtraPoints; k_p++)
        {
            //float a = 1.0f - float(k_p + 1) / float(ExtraPoints + 1);
            //float a = 1.0f - 1.0f / float(ExtraPoints - k_p + 1);
            
            //std::cout << "a: " << a << "\n";
            
            MidPoint = (PrevPoints[k*(ExtraPoints + 1) + k_p] + PrevPoints[k*(ExtraPoints + 1) + k_p + 1]
                        + PrevPoints[k*(ExtraPoints + 1) + k_p + 2])/3.0f;
            
            //SnapToCurve(MidPoint);
            //SnapToBary(MidPoint);
            SubdividedPoints[k*(ExtraPoints + 1) + k_p + 1] = MidPoint;
        }
        //SubdividedPoints = PrevPoints;
    }
    
    
    //SortArray(SubdividedPoints);


// 	std::cout << "------------\n";
// 	for (auto it = SubdividedPoints.rbegin(); it != SubdividedPoints.rend(); ++it)
// 	{
// 		std::cout << "point (" << (*it).x << "," << (*it).y << ")\n";
// 	}
// 	std::cout << "------------\n";
    
    LastUserPoints = SortedUserPoints;
}

void CurveDetect::SnapToCurve(ImVec2& ImagePoint)
{
    
    int localX, localY;//to this vars local coords will be written
    
    int hside = (int)SnapDistance;
    
    localX = int(ImagePoint.x);
    localY = int(ImagePoint.y);
    
    if (!image->getClosestBlack(localX, localY, hside, BinarizationLevel))
    {
        //std::cout << "Bad point!\n";
        return;
    }
    
    if (localX != int(ImagePoint.x))
    {
        ImagePoint.x = float(localX);
    }
    if (localY != int(ImagePoint.y))
    {
        ImagePoint.y = float(localY);
    }
    //std::cout << "loc snap: (" << snapX << "," << snapY << ")\n";
}

void CurveDetect::SnapToBary(ImVec2& ImagePoint)
{
    int localX, localY;//to this vars local coords will be written
    
    int baryX, baryY; //local coords of snapped point
    
    int hside = 3;// ZoomPixelHSide;
    
    localX = int(ImagePoint.x);
    localY = int(ImagePoint.y);
    
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
    
    ImVec2 BaryOffset;
    
    for (int kx = 0; kx <= 2 * hside; kx++)
    {
        for (int ky = 0; ky <= 2 * hside; ky++)
        {
            if (PointRegion(ky, kx) < BinarizationLevel)
            {
                baryMass += BinarizationLevel - PointRegion(ky, kx);
                
                BaryOffset += ImVec2(float(kx - localX), float(ky - localY))*float(BinarizationLevel - PointRegion(ky, kx));
                
            }
            
        }
    }
    
    if (baryMass > 0)
    {
        BaryOffset /= float(baryMass);
        
        ImagePoint += BaryOffset;
    }
    
    //ImagePoint.x += (baryX - localX);
    //ImagePoint.y += (baryY - localY);
}

ImVec2 CurveDetect::ConvertImageToReal(const ImVec2& ImagePoint)
{
    ImVec2 RealPoint;
    
    //this function should be called after check that
    //we can really calculate real points
    //so all x and y ticks are defined
    
    ImVec2 Scale, Offset;
    
    Scale.x = (XTicks[0].z - XTicks[1].z) / (XTicks[0].x - XTicks[1].x);
    Scale.y = (YTicks[0].z - YTicks[1].z) / (YTicks[0].y - YTicks[1].y);
    
    
    Offset.x = XTicks[1].z - XTicks[1].x*Scale.x;
    Offset.y = YTicks[1].z - YTicks[1].y*Scale.y;
    
    //XTicks[1].z + (ImagePoint.x- XTicks[1].x)*Scale.x
    
    RealPoint.x = Offset.x + ImagePoint.x*Scale.x;
    RealPoint.y = Offset.y + ImagePoint.y*Scale.y;
    
    //for deriving a and b
    float det1 = (XTicks[0].x - XTicks[1].x)*(CoordOriginTargetX.x - CoordOriginImg.x)
                 + (XTicks[0].y - XTicks[1].y)*(CoordOriginTargetX.y - CoordOriginImg.y);
    
    //for deriving c and d
    float det2 = (YTicks[0].x - YTicks[1].x)*(CoordOriginImg.y - CoordOriginTargetX.y)
                 - (YTicks[0].y - YTicks[1].y)*(CoordOriginImg.x - CoordOriginTargetX.x);
    
    float a, b, c, d, e, f;
    
    a = (XTicks[0].z - XTicks[1].z)*(CoordOriginTargetX.x - CoordOriginImg.x) / det1;
    b = (XTicks[0].z - XTicks[1].z)*(CoordOriginTargetX.y - CoordOriginImg.y) / det1;
    
    c = (YTicks[0].z - YTicks[1].z)*(CoordOriginImg.y - CoordOriginTargetX.y) / det2;
    d = -(YTicks[0].z - YTicks[1].z)*(CoordOriginImg.x - CoordOriginTargetX.x) / det2;
    
    e = XTicks[0].z - a*XTicks[0].x - b*XTicks[0].y;
    f = YTicks[0].z - c*YTicks[0].x - d*YTicks[0].y;
    
    RealPoint.x = a*ImagePoint.x + b*ImagePoint.y + e;
    RealPoint.y = c*ImagePoint.x + d*ImagePoint.y + f;
    
    
    return RealPoint;
}


void CurveDetect::ResetAll()
{
    UserPoints.clear();
    SortedUserPoints.clear();
    SubdividedPoints.clear();
    
    XTicks.clear();
    YTicks.clear();
    
    CoordOriginTargetX = CoordOriginImg + ImVec2(100.0f, 0.0f);
    
    
    if (image)
    {
        
        CoordOriginImg = ImVec2((float)image->get_width(), (float)image->get_height())*0.5f;
        CoordOriginTargetX = CoordOriginImg + ImVec2((float)image->get_width()*0.3f, 0.0f);
    }
    
}

void CurveDetect::AddPoint(ImVec2 pos)
{
    UserPoints.push_back(pos);
    SnapToCurve(UserPoints.back());
    SnapToBary(UserPoints.back());
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

void CurveDetect::SetOrigin(ImVec2 pos, bool snap)
{
//    CoordOriginTargetX += pos - CoordOriginImg;
    CoordOriginImg = pos;
    
//    SnappedPoint = CoordOriginImg;
    if (snap)
    {
        SnapToCurve(CoordOriginImg);
        SnapToBary(CoordOriginImg);
//        CoordOriginTargetX += SnappedPoint - CoordOriginImg;
        CoordOriginImg = CoordOriginImg;
    }
    
    SortPoints();
    UpdateSubdivision();
}

void CurveDetect::SetTarget(ImVec2 pos, bool snap)
{
    if (snap)
    {
        SnapToCurve(pos);
        SnapToBary(pos);
    }
    
    CoordOriginTargetX = pos;
    SortPoints();
    UpdateSubdivision();
}

bool CurveDetect::AddXTick(ImVec2 pos)
{
    if(XTicks.size()==2)
        return false;
    
    XTicks.push_back(ImVec4(pos.x, pos.y, 0.f, 0.f));
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

bool CurveDetect::AddYTick(ImVec2 pos)
{
    if(YTicks.size()==2)
        return false;
    
    YTicks.push_back(ImVec4(pos.x, pos.y, 0.f, 0.f));
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

void CurveDetect::MoveSelectedPoint(ImVec2 pos, bool snap)
{
    UserPoints[SelectedItem] = pos;
    if (snap)
    {
        SnapToCurve(UserPoints[SelectedItem]);
        SnapToBary(UserPoints[SelectedItem]);
    }
    SortPoints();
    UpdateSubdivision();
}

void CurveDetect::MoveSelectedXTick(ImVec2 pos, bool snap)
{
    if (snap)
    {
        SnapToCurve(pos);
        SnapToBary(pos);
    }
    XTicks[SelectedItem].x = pos.x;
    XTicks[SelectedItem].y = pos.y;
}

void CurveDetect::MoveSelectedYTick(ImVec2 pos, bool snap)
{
    if (snap)
    {
        SnapToCurve(pos);
        SnapToBary(pos);
    }
    YTicks[SelectedItem].x = pos.x;
    YTicks[SelectedItem].y = pos.y;
}

void CurveDetect::CheckTarget()
{
    if (std::abs(CoordOriginTargetX.x - CoordOriginImg.x) < 2.0f &&
        std::abs(CoordOriginTargetX.y - CoordOriginImg.y) < 2.0f)
    {
        CoordOriginTargetX = CoordOriginImg + ImVec2(10.0f, 0.0f);
    }
}

const std::vector<ImVec2> CurveDetect::GetAllPoints()
{
    return SubdividedPoints;
}

const std::vector<ImVec2> CurveDetect::GetUserPoints()
{
    return UserPoints;
}

std::vector<ImVec4>& CurveDetect::GetXTicks()
{
    return XTicks;
}

std::vector<ImVec4>& CurveDetect::GetYTicks()
{
    return YTicks;
}

ImVec2 CurveDetect::GetOrigin()
{
    return CoordOriginImg;
}

ImVec2 CurveDetect::GetTarget()
{
    return CoordOriginTargetX;
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

