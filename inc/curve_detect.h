#ifndef CURVEDETECT_CURVE_DETECT_H
#define CURVEDETECT_CURVE_DETECT_H

#include <vector>
#include <string>
#include <memory>
#include <image.h>
#include <image_elements.h>


class PointsBundle
{
public:
    ~PointsBundle();

    int allNum=0;
    int userNum=0;
    double* userPointsPixels=nullptr;
    double* userPointsReal=nullptr;
    double* allPointsPixels=nullptr;
    double* allPointsReal=nullptr;
};


class CurveDetect
{
public:

    enum ExportStatus //: int
    {
        READY =                0,
        NO_POINTS =            1 << 1,
        ONE_POINT =            1 << 2,
        PIXEL_OVERLAP_X_GRID = 1 << 3,
        VALUE_OVERLAP_X_GRID = 1 << 4,
        PIXEL_OVERLAP_Y_GRID = 1 << 5,
        VALUE_OVERLAP_Y_GRID = 1 << 6,
    };
    
    enum AxisScale
    {
        LINEAR  = 1,
        LOG  = 2,
    };

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
    void set_invert_image(bool invert);
    void set_subdiv_level(int subdiv);
    void set_curve_thickness(int thick);
    
    void set_scales(AxisScale xscale, AxisScale yscale);

    bool is_export_ready(int &out_Result);

    //TODO actual export should be inside main app?
    void export_points_mat_file(const char *path);
    std::string get_points_text(std::string columnSeparator, std::string lineEnding, char decimalSeparator);


    void update_subdiv(bool fullUpdate=false);

    
private:
    std::shared_ptr<Image> image;

    int binLevel;
    int subdivLevel;
    
    //TODO use snap distance (from image) instead
    float hoverZone = 14.f;
    //TODO use curve thickness instead
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


    AxisScale xscale = LINEAR;
    AxisScale yscale = LINEAR;

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

    std::shared_ptr<PointsBundle> get_points_bundle();
    
    void double_to_string(double num, char decimalSeparator, std::string &out_String);
};



#endif //CURVEDETECT_CURVE_DETECT_H
