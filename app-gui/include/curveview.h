#ifndef CURVEDETECT_CURVEVIEW_H
#define CURVEDETECT_CURVEVIEW_H

#include <memory>

#include <QWidget>

#include "curve_detect.h"
#include "mainwindow.h"

class CurveView : public QWidget
{
    Q_OBJECT
public:
    CurveView();
    ~CurveView() override;

    void setCurve(std::shared_ptr<CurveDetect> curve);

protected:

    void mousePressEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    void keyReleaseEvent(QKeyEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void enterEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
//    void timerEvent(QTimerEvent *event) override;
private:
    QPixmap image;
    std::shared_ptr<CurveDetect> curve;

    float imageScale;
    Vec2D imagePos;

    Vec2D hoveredImagePos;

    ActionMode currentMode;

    bool deleteOnRelease = false;
    bool bShowSubTicks = false;
    bool bLogX = false;
    bool bLogY = false;

    void drawImage(QPainter& painter);
    void drawPoints(QPainter& painter);
    void drawGrid(QPainter& painter);
    void drawHorizon(QPainter& painter);

    /**
     * Draw grid line that goes through specified point and has specified
     * direction. If value is not empty, draws it next to line
     * @param painter
     * @param point
     * @param dir
     * @param color
     * @param value
     */
    void drawGridLine(QPainter& painter, Vec2D point, Vec2D dir,
                      const QColor& color, const std::string& value = "");

    void openModePopup();

    Vec2D screen2image(Vec2D screenPos);
    Vec2D image2screen(Vec2D pos);

    bool extendLine(Vec2D point, Vec2D dir, Vec2D &out_Start,
                    Vec2D &out_End, Vec2D RegionSize, Vec2D RegionTL);
};

#endif //CURVEDETECT_CURVEVIEW_H
