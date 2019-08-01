#ifndef CURVEDETECT_CURVE_DETECT_H
#define CURVEDETECT_CURVE_DETECT_H

#include <vector>
#include <string>
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
    void ResetHorizon();
    void UpdateHoveredItem(Vec2D imagePos);

    void AddPoint(Vec2D pos);
    
    uint64_t GetSelectedId();
    ImageElement* GetSelected();
    uint64_t GetHoveredId(int selectionFilter);

    bool SelectHovered(int selectionFilter);
    void DeleteSelected();
    bool MoveSelected(Vec2D pos);
    void DeselectAll();
    void SnapSelected();
    void BackupSelectedTick();

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


    void UpdateSubdivision();

    static const int MaxSubdivideIterations=6;
    int SubdivideIterations;
    int BinarizationLevel;
    
private:
    std::shared_ptr<Image> image;
    
    float SnapDistance;
    float hoverZone = 14.f;
    float MinTickPixelDistance = 5.0f;
    float MinTickRealDistance = 0.001f;

    uint64_t hoveredPoint = 0;
    uint64_t selectedPoint = 0;

    uint64_t hoveredXtick = 0;
    uint64_t selectedXtick = 0;

    uint64_t hoveredYtick = 0;
    uint64_t selectedYtick = 0;

    ImageHorizon::HorizonPoint hoveredOrigin = ImageHorizon::NONE;
    ImageHorizon::HorizonPoint selectedOrigin = ImageHorizon::NONE;
    
    std::vector<ImagePoint> UserPoints; //position of user points (pixels)
    std::vector<ImagePoint> SubdividedPoints; //position of all points (pixels) both user and generated
    int subdivThreshold = -1;

    std::vector<ImageTickLine> XTicks;
    std::vector<ImageTickLine> YTicks;

    ImageHorizon horizon;

    void UpdateHoveredPoint(Vec2D imagePos);
    void UpdateHoveredTickX(Vec2D imagePos);
    void UpdateHoveredTickY(Vec2D imagePos);
    void UpdateHoveredHorizon(Vec2D imagePos);

    void SortPoints();
    void SortArray(std::vector<ImagePoint>& Array);
    bool IsArraySorted(std::vector<ImagePoint>& Array);

    
    /**
     * will snap point to closest black (0) point
     * so for more precision use with SnapToBary
     * @param ImagePoint
     */
    bool SnapToCurve(Vec2D& point);
    
    /**
     * will snap point to barycenter of current zoom region
     * will not be precise if there are grid lines in the region
     * so make sure that in zoom region only curve points are shown
     * @param ImagePoint
     */
    bool SnapToBary(Vec2D& point);
    
    /**
     * converts pixel coordinates to real data
     * @param ImagePoint
     * @return
     */
    Vec2D ConvertImageToReal(const Vec2D& point);
    
    
    void numToStr(double num, char decimalSeparator, std::string& out_String);
};



#endif //CURVEDETECT_CURVE_DETECT_H
