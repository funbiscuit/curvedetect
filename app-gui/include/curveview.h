#ifndef CURVEDETECT_CURVEVIEW_H
#define CURVEDETECT_CURVEVIEW_H

#include <QWidget>

class CurveView : public QWidget
{
    Q_OBJECT
public:
    CurveView();
    ~CurveView() override;


protected:

//    void mousePressEvent(QMouseEvent *event) override;

//    void mouseReleaseEvent(QMouseEvent *event) override;
//    void mouseMoveEvent(QMouseEvent *event) override;
//    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
//    void timerEvent(QTimerEvent *event) override;

};

#endif //CURVEDETECT_CURVEVIEW_H
