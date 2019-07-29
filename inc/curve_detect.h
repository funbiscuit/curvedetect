#ifndef CURVEDETECT_CURVE_DETECT_H
#define CURVEDETECT_CURVE_DETECT_H

#include <vector>
#include <string>
//TODO should not be here, use something else for ImVec2 and ImVec4
#include <imgui.h>
#include <memory>
#include <image.h>
#include <image_elements.h>


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
    void UpdateHoveredItemIndex(Vec2D im_pos, int mode);
    
    void AddPoint(Vec2D pos);
    int GetHoveredPoint();
    void DeleteHoveredPoint();
    void SelectHovered();
    
    void SetOrigin(Vec2D pos, bool snap);
    void SetTarget(Vec2D pos, bool snap);

    bool AddXTick(Vec2D pos);
    int GetHoveredXTick();
    void DeleteHoveredXTick();
    
    bool AddYTick(Vec2D pos);
    int GetHoveredYTick();
    void DeleteHoveredYTick();
    
    int GetSelected();
    int GetHovered();
    
    void MoveSelectedPoint(Vec2D pos, bool snap);
    void MoveSelectedXTick(Vec2D pos, bool snap);
    void MoveSelectedYTick(Vec2D pos, bool snap);

    void Deselect(){SelectedItem=-1;}
    
    void CheckTarget();
    
    const std::vector<ImagePoint> GetAllPoints();
    const std::vector<ImagePoint> GetUserPoints();
    std::vector<ImageTickLine>& GetXTicks();
    std::vector<ImageTickLine>& GetYTicks();

    ImageHorizon GetHorizon();
    
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
    
    std::vector<ImagePoint> UserPoints; //position of user points (pixels)
    std::vector<ImagePoint> SortedUserPoints; //position of user points (pixels)
    std::vector<ImagePoint> SubdividedPoints; //position of all points (pixels) both user and generated

    std::vector<ImageTickLine> XTicks;
    std::vector<ImageTickLine> YTicks;

    ImageHorizon horizon;

    void SortPoints();
    
    void SortArray(std::vector<ImagePoint>& Array);
    
    
    void UpdateSubdivision(bool bUpdateAll = false);
    
    /**
     * will snap point to closest black (0) point
     * so for more precision use with SnapToBary
     * @param ImagePoint
     */
    void SnapToCurve(Vec2D& point);
    
    /**
     * will snap point to barycenter of current zoom region
     * will not be precise if there are grid lines in the region
     * so make sure that in zoom region only curve points are shown
     * @param ImagePoint
     */
    void SnapToBary(Vec2D& point);
    
    /**
     * converts pixel coordinates to real data
     * @param ImagePoint
     * @return
     */
    Vec2D ConvertImageToReal(const Vec2D& point);
    
    
    void numToStr(double num, char decimalSeparator, std::string& out_String);
};



#endif //CURVEDETECT_CURVE_DETECT_H
