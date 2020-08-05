#include <iostream>

#include <QtWidgets>

#include "curveview.h"


CurveView::CurveView()
{
    setMouseTracking(true);

    currentMode = ActionMode::MODE_POINTS;
}

CurveView::~CurveView()
{

}

void CurveView::setCurve(std::shared_ptr<CurveDetect> _curve)
{
    curve = std::move(_curve);
    image = curve->getImage()->getPixmap();
}

void CurveView::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {

        switch (currentMode)
        {
            case MODE_POINTS:
                //TODO
                if (true) // io.KeyCtrl
                    curve->add_point(screen2image(Vec2D(event->localPos())));
                else
                    curve->select_hovered(ImageElement::POINT);
                break;
            case MODE_HORIZON:
                curve->select_hovered(ImageElement::HORIZON);
                break;
            case MODE_GRID:
                if (curve->select_hovered(ImageElement::TICKS))
                    curve->backup_selected_tick();
                break;
            default:
                break;
        }
        repaint();
    }
}

void CurveView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        switch (currentMode)
        {
            case MODE_POINTS:
            case MODE_HORIZON:
                //TODO
                if (false) // deleteOnRelease
                    curve->delete_selected();
                else
                    curve->deselect_all();
                break;
            case MODE_GRID:
                curve->deselect_all();
                break;
            default:
                break;
        }
        repaint();
    }
}

void CurveView::mouseMoveEvent(QMouseEvent *event)
{
    auto hoveredImagePixel = (Vec2D(event->x(),event->y()) - imagePos)/imageScale;

    if(curve)
    {
        auto prevHovered = curve->get_hovered_id(ImageElement::ALL);
        curve->update_hovered(hoveredImagePixel);
        if(curve->get_hovered_id(ImageElement::ALL) != prevHovered)
            repaint();
    }
}

void CurveView::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    auto width = float(size().width());
    auto height = float(size().height());

    painter.setPen(QColor(Qt::gray));
    painter.drawRect(0, 0, size().width()-1, size().height()-1);

    if(!curve || image.isNull())
        return;

    auto imWidth = float(image.width());
    auto imHeight = float(image.height());

    float scaleX = width / imWidth;
    float scaleY = height / imHeight;

    imageScale = std::min(scaleX, scaleY);

    imagePos.x = 1 + (width - imageScale * imWidth) / 2.f;
    imagePos.y = 1 + (height - imageScale * imHeight) / 2.f;

    painter.drawPixmap(
            QRectF(imagePos.x, imagePos.y,
                    imageScale*imWidth-2, imageScale*imHeight-2), image,
            QRectF(image.rect()));

    drawPoints(painter);
}

void CurveView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}

void CurveView::drawPoints(QPainter& painter)
{
    if(!curve)
        return;

    painter.setRenderHint(QPainter::RenderHint::Antialiasing);

    uint64_t selectedId= curve->get_selected_id();
    uint64_t hoveredId= curve->get_hovered_id(ImageElement::POINT);
    auto& segments = curve->get_segments();
    auto& userPoints = curve->get_user_points();

    float subdivSize = 8.f;
    float userSize = 12.f;
    QColor lineColor(128, 128, 128, 255);
    QColor pointStroke(64, 64, 64, 255);
    QColor userHover(255, 255, 255, 255);
    QColor userFill(124, 252, 0, 255);
    QColor deleteFill(205, 92, 92, 255);
    QColor subdivFill(128, 128, 128, 255);
    QColor subdivBadFill = deleteFill;

    double subdivSpacing = subdivSize;

    //draw lines between subdivided points
    if (!segments.empty())
    {
        for(auto& segment : segments)
        {
            auto pointPos0 = segment.begin.imagePosition * imageScale + imagePos;

            bool prevSnapped = true;
            for(size_t i=1;i<segment.points.size();++i)
            {
                const ImagePoint& point = segment.points[i];
                auto pointPos1 = point.imagePosition * imageScale + imagePos;

                bool snapped = point.isSnapped || !point.isSubdivisionPoint;
                auto stroke = (prevSnapped && snapped) ? lineColor : subdivBadFill;

                painter.setPen(stroke);
                painter.drawLine(pointPos0.toQPoint(), pointPos1.toQPoint());
                pointPos0=pointPos1;
                prevSnapped=snapped;
            }
        }

        //used to limit number of drawn points (so points are not drawn on top of each other)
        Vec2D lastDrawnPos = segments[0].begin.imagePosition * imageScale + imagePos;
        //if (bShowSubdivPoints)
        {
            for (auto &segment : segments) {
                for (size_t i = 1; i < segment.points.size(); ++i) {
                    const ImagePoint &point = segment.points[i];
                    auto PointPos1 = point.imagePosition * imageScale + imagePos;

                    auto fill = point.isSnapped ? subdivFill : subdivBadFill;

                    double dist1 = Vec2D(PointPos1 - lastDrawnPos).norm2();

                    if (point.isSubdivisionPoint && (dist1 > subdivSpacing * subdivSpacing))
                    {
                        painter.setPen(pointStroke);
                        painter.setBrush(QBrush(fill));
                        painter.drawEllipse(PointPos1.toQPoint(), subdivSize*0.5, subdivSize*0.5);
                        lastDrawnPos = PointPos1;
                    }
                }

            }
        }
    }

    for (const auto &point : userPoints) {
        auto pointPos = point.imagePosition * imageScale + imagePos;

        auto fill = userFill;

        //TODO
        if (currentMode == MODE_POINTS)
            if (point.id == selectedId || (selectedId == 0 && point.id == hoveredId))
//                fill = deleteOnRelease ? deleteFill : userHover;
                fill = userHover;

        painter.setPen(pointStroke);
        painter.setBrush(QBrush(fill));
        painter.drawEllipse(pointPos.toQPoint(), userSize * 0.5, userSize * 0.5);
    }


}

Vec2D CurveView::screen2image(Vec2D screenPos)
{
    return (screenPos - imagePos) / imageScale;
}
