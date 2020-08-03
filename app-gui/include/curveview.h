#ifndef CURVEDETECT_CURVEVIEW_H
#define CURVEDETECT_CURVEVIEW_H

#include <memory>

#include <QWidget>

#include "curve_detect.h"

class CurveView : public QWidget
{
    Q_OBJECT
public:
    CurveView();
    ~CurveView() override;

    void setCurve(std::shared_ptr<CurveDetect> curve);

protected:

//    void mousePressEvent(QMouseEvent *event) override;

//    void mouseReleaseEvent(QMouseEvent *event) override;
//    void mouseMoveEvent(QMouseEvent *event) override;
//    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
//    void timerEvent(QTimerEvent *event) override;
private:
    QPixmap image;
    std::shared_ptr<CurveDetect> curve;

    float imageScale;
    Vec2D imagePos;

    void drawPoints(QPainter& painter);
};

#endif //CURVEDETECT_CURVEVIEW_H
