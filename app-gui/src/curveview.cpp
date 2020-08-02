#include <QPainter>

#include "curveview.h"


CurveView::CurveView()
{

}

CurveView::~CurveView()
{

}

void CurveView::setImage(const Image& _image)
{
    image = _image.getPixmap();
}

void CurveView::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    auto width = float(size().width());
    auto height = float(size().height());

    painter.setPen(QColor(Qt::gray));
    painter.drawRect(0, 0, size().width()-1, size().height()-1);

    if(image.isNull())
        return;

    auto imWidth = float(image.width());
    auto imHeight = float(image.height());

    float scaleX = width / imWidth;
    float scaleY = height / imHeight;

    float imageScale = std::min(scaleX, scaleY);

    float imX = (width - imageScale * imWidth) / 2.f;
    float imY = (height - imageScale * imHeight) / 2.f;

    painter.drawPixmap(
            QRectF(1+imX, 1+imY,
                    imageScale*imWidth-2, imageScale*imHeight-2), image,
            QRectF(image.rect()));

}

void CurveView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}
