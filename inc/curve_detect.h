#ifndef CURVEDETECT_CURVE_DETECT_H
#define CURVEDETECT_CURVE_DETECT_H

#include <vector>
#include <string>
#include <memory>
#include <image.h>
#include <image_elements.h>


enum ExportReadyStatus //: int
{
    READY =                1 << 0,
    NO_IMAGE =             1 << 1,
    NO_POINTS =            1 << 2,
    ONE_POINT =            1 << 3,
    NO_X_GRID_LINES =      1 << 4,
    ONE_X_GRID_LINE =      1 << 5,
    NO_Y_GRID_LINES =      1 << 6,
    ONE_Y_GRID_LINE =      1 << 7,
    PIXEL_OVERLAP_X_GRID = 1 << 8,
    VALUE_OVERLAP_X_GRID = 1 << 9,
    PIXEL_OVERLAP_Y_GRID = 1 << 10,
    VALUE_OVERLAP_Y_GRID = 1 << 11,

};


class CurveDetect
{
public:
    
    CurveDetect(std::shared_ptr<Image> image);
    
    void reset_all();
    void reset_horizon();
    void update_hovered(Vec2D hoveredImagePos);

    void add_point(Vec2D pos);
    
    uint64_t get_selected_id();
    ImageElement* get_selected();
    uint64_t get_hovered_id(int selectionFilter);

    bool select_hovered(int selectionFilter);
    void delete_selected();
    bool move_selected(Vec2D pos);
    void deselect_all();
    void snap_selected();
    void backup_selected_tick();

    void check_horizon();
    
    const std::vector<ImagePoint> get_all_points();
    const std::vector<ImagePoint> get_user_points();
    std::vector<ImageTickLine>& get_xticks();
    std::vector<ImageTickLine>& get_yticks();

    ImageHorizon get_horizon();
    
    void set_bin_level(int level);
    void set_subdiv_level(int subdiv);
    
    bool is_export_ready(int &out_Result);

    //TODO actual export should be inside main app?
    void export_points(const char *path, bool asText);
    void copy_to_clipboard(std::string columnSeparator, std::string lineEnding, char decimalSeparator);


    void update_subdiv();

    static const int maxSubdivLevel=8;
    int subdivLevel;
    int binLevel;
    
private:
    std::shared_ptr<Image> image;

    //TODO move to settings
    float snapDist;
    float hoverZone = 14.f;
    double minTickPixelDiff = 5.0;

    uint64_t hoveredPoint = 0;
    uint64_t selectedPoint = 0;

    uint64_t hoveredXtick = 0;
    uint64_t selectedXtick = 0;

    uint64_t hoveredYtick = 0;
    uint64_t selectedYtick = 0;

    ImageHorizon::HorizonPoint hoveredOrigin = ImageHorizon::NONE;
    ImageHorizon::HorizonPoint selectedOrigin = ImageHorizon::NONE;
    
    std::vector<ImagePoint> userPoints; //position of user points (pixels)
    std::vector<ImagePoint> allPoints; //position of all points (pixels) both user and generated

    // binarization level that was used to calculate current subdivision
    // so if it didn't change we can update subdivision much faster
    int subdivBinLevel = -1;

    std::vector<ImageTickLine> xticks;
    std::vector<ImageTickLine> yticks;

    ImageHorizon horizon;

    void update_hovered_point(Vec2D imagePos);
    void update_hovered_tickx(Vec2D imagePos);
    void update_hovered_ticky(Vec2D imagePos);
    void update_hovered_horizon(Vec2D imagePos);

    void sort_points();
    bool are_points_sorted();

    bool snap(Vec2D &pos);

    /**
     * converts pixel coordinates to real data
     * @param ImagePoint
     * @return
     */
    Vec2D image_point_to_real(const Vec2D &point);
    
    
    void double_to_string(double num, char decimalSeparator, std::string &out_String);
};



#endif //CURVEDETECT_CURVE_DETECT_H
