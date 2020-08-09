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
            case ActionMode::MODE_POINTS:
                if (QApplication::keyboardModifiers() & Qt::ControlModifier)
                    curve->add_point(screen2image(Vec2D(event->localPos())));
                else
                    curve->select_hovered(ImageElement::POINT);
                break;
            case ActionMode::MODE_HORIZON:
                curve->select_hovered(ImageElement::HORIZON);
                break;
            case ActionMode::MODE_GRID:
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
            case ActionMode::MODE_POINTS:
            case ActionMode::MODE_HORIZON:
                if (deleteOnRelease)
                    curve->delete_selected();
                else
                    curve->deselect_all();
                break;
            case ActionMode::MODE_GRID:
                curve->deselect_all();
                break;
            default:
                break;
        }
        repaint();
    } else if (event->button() == Qt::RightButton)
    {
        curve->deselect_all();
        repaint();
        openModePopup();
    }
}

void CurveView::mouseMoveEvent(QMouseEvent *event)
{
    if(!curve)
        return;

    bool redraw = false;

    hoveredImagePos = screen2image(Vec2D(event->localPos()));
    auto prevHovered = curve->get_hovered_id(ImageElement::ALL);
    curve->update_hovered(hoveredImagePos);
    if(curve->get_hovered_id(ImageElement::ALL) != prevHovered)
        redraw = true;

    if(curve->move_selected(hoveredImagePos))
    {
        if(QApplication::keyboardModifiers() & Qt::ControlModifier)
            curve->snap_selected();

        if(currentMode == ActionMode::MODE_POINTS ||
           currentMode == ActionMode::MODE_HORIZON)
        {
            curve->update_subdiv();
        }
        redraw = true;
    }

    if(redraw)
        repaint();
}

void CurveView::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    // draw frame for curve view
    painter.setPen(QColor(Qt::gray));
    painter.drawRect(0, 0, size().width()-1, size().height()-1);

    // nothing else to do if image is not loaded
    if(!curve || image.isNull())
        return;

    drawImage(painter);

    drawPoints(painter);

    drawGrid(painter);

    drawHorizon(painter);
}

void CurveView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}

void CurveView::drawImage(QPainter& painter)
{
    auto width = float(size().width());
    auto height = float(size().height());

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
}

void CurveView::drawPoints(QPainter& painter)
{
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

        if (currentMode == ActionMode::MODE_POINTS)
            if (point.id == selectedId || (selectedId == 0 && point.id == hoveredId))
                fill = deleteOnRelease ? deleteFill : userHover;

        painter.setPen(pointStroke);
        painter.setBrush(QBrush(fill));
        painter.drawEllipse(pointPos.toQPoint(), userSize * 0.5, userSize * 0.5);
    }


}

void CurveView::drawGrid(QPainter& painter)
{
    const auto tickColor = QColor(128, 128, 128);
    const auto tickHover = QColor(152, 248, 59);
    const auto tickSel = QColor(59, 155, 59);


    auto horizon= curve->get_horizon();
    auto CoordOriginImg= horizon.imagePosition;
    auto CoordOriginTargetX= horizon.target.imagePosition;
    uint64_t hoveredId= curve->get_hovered_id(ImageElement::TICKS);
    uint64_t selectedId= curve->get_selected_id();
    auto& XTicks= curve->get_xticks();
    auto& YTicks= curve->get_yticks();

    auto TargetDirX = CoordOriginTargetX - CoordOriginImg;
    TargetDirX/=std::sqrt(TargetDirX.x*TargetDirX.x + TargetDirX.y*TargetDirX.y);

    Vec2D TargetDirY;
    TargetDirY.x = TargetDirX.y;
    TargetDirY.y = -TargetDirX.x;

    if(bShowSubTicks)
    {
        int N=10;
        auto begin = XTicks[0].imagePosition;
        auto end = XTicks[1].imagePosition;
        for(int j=1; j<N; ++j)
        {
            double v = bLogX ? std::log(j)/std::log(10.0) : double(j)/N;
            if(XTicks[0].tickValue>XTicks[1].tickValue)
                v=1.0-v;
            auto pos = begin + (end-begin)*v;

            drawGridLine(painter, pos, TargetDirY, tickColor);
        }
        begin = YTicks[0].imagePosition;
        end = YTicks[1].imagePosition;
        for(int j=1; j<N; ++j)
        {
            double v = bLogY ? std::log(j)/std::log(10.0) : double(j)/N;
            if(YTicks[0].tickValue>YTicks[1].tickValue)
                v=1.0-v;
            auto pos = begin + (end-begin)*v;

            drawGridLine(painter, pos, TargetDirX, tickColor);
        }
    }

    //draw tick lines
    for (auto &tick : XTicks) {
        auto col = tickColor;

        if (currentMode == ActionMode::MODE_GRID)
        {
            if (tick.id == selectedId)
                col = tickSel;
            else if (tick.id == hoveredId && !selectedId)
                col = tickHover;
        }

        drawGridLine(painter, tick.imagePosition,
                         TargetDirY, col, tick.tickValueStr);
    }
    for (auto &tick : YTicks) {
        auto col = tickColor;

        if (currentMode == ActionMode::MODE_GRID)
        {
            if (tick.id == selectedId)
                col = tickSel;
            else if (tick.id == hoveredId && !selectedId)
                col = tickHover;
        }

        drawGridLine(painter, tick.imagePosition,
                         TargetDirX, col, tick.tickValueStr);
    }
}

void CurveView::drawHorizon(QPainter& painter)
{
    if(currentMode != ActionMode::MODE_HORIZON)
        return;

    auto horizon = curve->get_horizon();
    auto selectedId = curve->get_selected_id();
    auto hoveredId = curve->get_hovered_id(ImageElement::HORIZON);

    float pointSize = 12.f;
    QColor lineColor(128, 128, 128);
    QColor pointStroke(64, 64, 64);
    QColor pointHover(255, 255, 255);
    QColor pointFill(255, 200, 60);
    QColor deleteFill(205, 92, 92);

    auto origin = horizon.imagePosition;
    auto target = horizon.target.imagePosition;

    auto originScr = image2screen(origin).toQPoint();
    auto targetScr = image2screen(target).toQPoint();

    //draw horizon
    painter.setPen(QPen(QBrush(lineColor), 2.f));
    painter.drawLine(originScr, targetScr);

    auto fill = pointFill;

    if (ImageHorizon::ORIGIN == selectedId ||
        (selectedId == 0 && ImageHorizon::ORIGIN == hoveredId))
        fill = deleteOnRelease ? deleteFill : pointHover;

    painter.setPen(pointStroke);
    painter.setBrush(QBrush(fill));
    painter.drawEllipse(originScr, pointSize * 0.5, pointSize * 0.5);

    fill = pointFill;

    if (ImageHorizon::TARGET == selectedId ||
        (selectedId == 0 && ImageHorizon::TARGET == hoveredId))
        fill = deleteOnRelease ? deleteFill : pointHover;

    painter.setPen(pointStroke);
    painter.setBrush(QBrush(fill));
    painter.drawEllipse(targetScr, pointSize * 0.5, pointSize * 0.5);
}

void CurveView::drawGridLine(QPainter& painter, Vec2D point, Vec2D dir,
                             const QColor& color, const std::string& value)
{
    Vec2D LineStart, LineEnd, LabelPos;

    auto LineMargin = Vec2D(10.0f, 10.0f);

    float lineThick = value.empty() ? 1.f : 2.f;

    if (!extendLine(point, dir, LineStart, LineEnd,
                     Vec2D((float) image.width(), (float) image.height()) - LineMargin * 2, LineMargin))
    {
        LineStart = Vec2D(LineMargin.x, point.y);
        LineEnd = Vec2D(image.width() - LineMargin.x, point.y);
    }
    LineStart = imagePos + LineStart*imageScale;
    LineEnd = imagePos + LineEnd*imageScale;

    painter.setPen(QPen(QBrush(color), lineThick));
    painter.drawLine(LineStart.toQPoint(), LineEnd.toQPoint());

    if(!value.empty())
    {
        auto f = painter.font();
        f.setPointSize(13);
        painter.setFont(f);
        auto sz = painter.fontMetrics().size(Qt::TextSingleLine, value.c_str());
        sz = sz.grownBy(QMargins(5,5,5,5));

        LabelPos = image2screen(point);
        LabelPos.x -= sz.width()/2;
        LabelPos.y -= sz.height()/2;
        QRectF labelRect(LabelPos.toQPoint(), sz);
        painter.fillRect(labelRect, QColor(255, 255, 255));

        painter.setPen(QColor(0,0,0));
        painter.drawText(labelRect, Qt::AlignCenter, value.c_str());
    }
}

void CurveView::openModePopup()
{
    QMenu menu(this);
    auto titleAction = menu.addAction("Choose mode:");
    titleAction->setEnabled(false);
    menu.addSeparator();


    const char* items[]={"[1] Points","[2] Grid","[3] Horizon"};
    ActionMode modes[]={ActionMode::MODE_POINTS,ActionMode::MODE_GRID,
                        ActionMode::MODE_HORIZON};

    for(int j=0;j<3;++j)
    {
        auto action = new QAction(items[j], this);
        connect(action, &QAction::triggered, this, [j, modes, this]()
        {
            currentMode = modes[j];
        });
        if(currentMode == modes[j])
            action->setDisabled(true);
        menu.addAction(action);
    }

    menu.exec(QCursor::pos()+QPoint(10,10));
}

Vec2D CurveView::screen2image(Vec2D screenPos)
{
    return (screenPos - imagePos) / imageScale;
}

Vec2D CurveView::image2screen(Vec2D pos)
{
    return imagePos + pos * imageScale;
}

bool CurveView::extendLine(Vec2D point, Vec2D dir, Vec2D &out_Start,
                           Vec2D &out_End, Vec2D RegionSize, Vec2D RegionTL)
{
    out_Start = point;
    out_End = point + dir;


    if (std::abs(dir.x) + std::abs(dir.y)<0.0001f)
    {
        return false;
    }

    float A = dir.y;
    float B = -dir.x;
    float C = -A*point.x - B*point.y;
    float C2 = B*point.x - A*point.y;

    auto RegionBR = RegionTL + RegionSize;//bottom-right corner

    //x line: A*x+B*y+C=0		C=-A*x0-B*y0
    //y line: -B*x+A*y+C2=0;	C2=B*x0-A*y0

    float lefty, righty, topx, botx;

    righty = (-C - A *RegionBR.x) / B;
    lefty = (-C - A *RegionTL.x) / B;
    botx = (-C - B *RegionBR.y) / A;
    topx = (-C - B *RegionTL.y) / A;

    out_Start = Vec2D(RegionTL.x, lefty);

    if (lefty<RegionTL.y)
        out_Start = Vec2D(topx, RegionTL.y);
    if (lefty>RegionBR.y)
        out_Start = Vec2D(botx, RegionBR.y);

    out_End = Vec2D(RegionBR.x, righty);

    if (righty<RegionTL.y)
        out_End = Vec2D(topx, RegionTL.y);
    if (righty>RegionBR.y)
        out_End = Vec2D(botx, RegionBR.y);

    if (B > 0.0f)
        std::swap(out_Start, out_End);

    return true;
}

void CurveView::keyPressEvent(QKeyEvent *event)
{
    if(!curve)
        return;

    if(event->key() == Qt::Key_Control)
    {
        curve->snap_selected();
        curve->update_subdiv();
        repaint();
    } else if(event->key() == Qt::Key_Shift)
    {
        deleteOnRelease = curve->get_selected_id() == 0;
        repaint();
    }

}

void CurveView::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Control)
    {
        curve->move_selected(hoveredImagePos);
        curve->update_subdiv();
        repaint();
    } else if(event->key() == Qt::Key_Shift)
    {
        deleteOnRelease = false;
        repaint();
    }

}

void CurveView::enterEvent(QEvent *event) {
    QWidget::enterEvent(event);
    grabKeyboard();
}

void CurveView::leaveEvent(QEvent *event) {
    QWidget::enterEvent(event);
    releaseKeyboard();
    deleteOnRelease = false;
}
