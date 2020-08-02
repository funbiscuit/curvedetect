#ifndef CURVEDETECT_CURVEVIEW_H
#define CURVEDETECT_CURVEVIEW_H

#include <QWidget>

#include "image.h"

class CurveView : public QWidget
{
    Q_OBJECT
public:
    CurveView();
    ~CurveView() override;

    void setImage(const Image& image);

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
};

#endif //CURVEDETECT_CURVEVIEW_H
