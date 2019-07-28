#ifndef CURVEDETECT_CURVE_DETECT_H
#define CURVEDETECT_CURVE_DETECT_H

#include <vector>
#include <string>
//TODO should not be here, use something else for ImVec2 and ImVec4
#include <imgui.h>
#include <memory>
#include <image.h>


enum ExportReadyStatus //: int
{
    ExportReadyStatus_Ready = 0,
    ExportReadyStatus_NoPoints = 1,
    ExportReadyStatus_NoXTicks = 2,
    ExportReadyStatus_NoYTicks = 4,
    ExportReadyStatus_XTicksSimilarPositions = 8,
    ExportReadyStatus_XTicksSimilarValues = 16,
    ExportReadyStatus_YTicksSimilarPositions = 32,
    ExportReadyStatus_YTicksSimilarValues = 64,
    
};


class CurveDetect
{
public:
    
    CurveDetect(std::shared_ptr<Image> image);
    
    void ResetAll();
    void UpdateHoveredItemIndex(ImVec2 im_pos, int mode);
    
    void AddPoint(ImVec2 pos);
    int GetHoveredPoint();
    void DeleteHoveredPoint();
    void SelectHovered();
    
    void SetOrigin(ImVec2 pos, bool snap);
    void SetTarget(ImVec2 pos, bool snap);

    bool AddXTick(ImVec2 pos);
    int GetHoveredXTick();
    void DeleteHoveredXTick();
    
    bool AddYTick(ImVec2 pos);
    int GetHoveredYTick();
    void DeleteHoveredYTick();
    
    int GetSelected();
    int GetHovered();
    
    void MoveSelectedPoint(ImVec2 pos, bool snap);
    void MoveSelectedXTick(ImVec2 pos, bool snap);
    void MoveSelectedYTick(ImVec2 pos, bool snap);

    void Deselect(){SelectedItem=-1;}
    
    void CheckTarget();
    
    const std::vector<ImVec2> GetAllPoints();
    const std::vector<ImVec2> GetUserPoints();
    std::vector<ImVec4>& GetXTicks();
    std::vector<ImVec4>& GetYTicks();
    
    ImVec2 GetOrigin();
    ImVec2 GetTarget();
    
    void SetBinarizationLevel(int level);
    void SetSubdivIterations(int subdiv);
    
    bool IsReadyForExport(int& out_Result);
    
    void ExportPoints(const char* path, bool asText);
    void ExportToClipboard(std::string columnSeparator, std::string lineEnding, char decimalSeparator);
    
    static const int MaxSubdivideIterations=6;
    int SubdivideIterations;
    int BinarizationLevel;
    
private:
    std::shared_ptr<Image> image;
    
    float SnapDistance;
    float MinTickPixelDistance = 5.0f;
    float MinTickRealDistance = 0.001f;
    
    
    int SelectedItem;
    int HoveredItem;
    
    std::vector<ImVec2> UserPoints; //position of user points (pixels)
    std::vector<ImVec2> SortedUserPoints; //position of user points (pixels)
    std::vector<ImVec2> SubdividedPoints; //position of all points (pixels) both user and generated
    
    
    std::vector<ImVec4> XTicks;	//first two - image coordinates, third - real X value, fourth - not used
    std::vector<ImVec4> YTicks;	//first two - image coordinates, third - real X value, fourth - not used
    
    ImVec2 CoordOriginImg; // position of origin in image (pixels)
    ImVec2 CoordOriginTargetX; // position of target in image (pixels) where X is pointed
    
    
    void SortPoints();
    
    void SortArray(std::vector<ImVec2>& Array);
    
    
    void UpdateSubdivision(bool bUpdateAll = false);
    
    /**
     * will snap point to closest black (0) point
     * so for more precision use with SnapToBary
     * @param ImagePoint
     */
    void SnapToCurve(ImVec2& ImagePoint);
    
    /**
     * will snap point to barycenter of current zoom region
     * will not be precise if there are grid lines in the region
     * so make sure that in zoom region only curve points are shown
     * @param ImagePoint
     */
    void SnapToBary(ImVec2& ImagePoint);
    
    /**
     * converts pixel coordinates to real data
     * @param ImagePoint
     * @return
     */
    ImVec2 ConvertImageToReal(const ImVec2& ImagePoint);
    
    
    void numToStr(float num, char decimalSeparator, std::string& out_String);
};



#endif //CURVEDETECT_CURVE_DETECT_H
