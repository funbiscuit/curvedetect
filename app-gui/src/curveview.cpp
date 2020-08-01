#include <QPainter>

#include "curveview.h"


CurveView::CurveView()
{

}

CurveView::~CurveView()
{

}

void CurveView::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    int width = size().width();
    int height = size().height();

    painter.setPen(QColor(Qt::gray));
    painter.drawRect(0, 0, width-1, height-1);

}

void CurveView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}
