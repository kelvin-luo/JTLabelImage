#include "CanvasWidget.h"

#include <QKeyEvent>
#include <QLineF>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
#include <QWheelEvent>
#include <QtMath>

CanvasWidget::CanvasWidget(QWidget* parent) : QWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, m_themeBg);
    setPalette(pal);
}

void CanvasWidget::setThemeColors(const QColor& bg,
                                  const QColor& hint,
                                  const QColor& checkerDark,
                                  const QColor& checkerLight) {
    m_themeBg = bg;
    m_themeHint = hint;
    m_checkerDark = checkerDark;
    m_checkerLight = checkerLight;
    QPalette pal = palette();
    pal.setColor(QPalette::Window, m_themeBg);
    setPalette(pal);
    update();
}

void CanvasWidget::setImage(const QImage& img, const QString& path) {
    m_image = img;
    m_imagePath = path;
    m_shapes.clear();
    m_selected = -1;
    m_drawing = false;
    m_stage = 0;
    fitToView();
    update();
    emit shapesChanged();
    emit selectionChanged(-1);
}

void CanvasWidget::setShapes(const QVector<Shape>& shapes) {
    m_shapes = shapes;
    m_selected = -1;
    update();
    emit shapesChanged();
    emit selectionChanged(-1);
}

void CanvasWidget::setTool(Tool t) {
    m_tool = t;
    m_drawing = false;
    m_stage = 0;
    m_current = Shape();
    update();
}

void CanvasWidget::setCurrentLabel(const QString& label, const QColor& color) {
    m_label = label;
    m_color = color;
}

void CanvasWidget::clearShapes() {
    m_shapes.clear();
    m_selected = -1;
    update();
    emit shapesChanged();
    emit selectionChanged(-1);
}

void CanvasWidget::deleteSelected() {
    if (m_selected < 0 || m_selected >= m_shapes.size()) return;
    m_shapes.removeAt(m_selected);
    m_selected = -1;
    update();
    emit shapesChanged();
    emit selectionChanged(-1);
}

void CanvasWidget::setSelected(int idx) {
    m_selected = (idx >= 0 && idx < m_shapes.size()) ? idx : -1;
    update();
    emit selectionChanged(m_selected);
}

void CanvasWidget::fitToView() {
    if (m_image.isNull() || width() <= 0 || height() <= 0) {
        m_zoom = 1.0; m_offset = {0, 0}; return;
    }
    const double zx = double(width())  / m_image.width();
    const double zy = double(height()) / m_image.height();
    m_zoom = qMin(zx, zy) * 0.95;
    m_offset.setX((width()  - m_image.width()  * m_zoom) / 2.0);
    m_offset.setY((height() - m_image.height() * m_zoom) / 2.0);
}

QPointF CanvasWidget::widgetToImage(const QPointF& p) const {
    return (p - m_offset) / m_zoom;
}

QPointF CanvasWidget::imageToWidget(const QPointF& p) const {
    return p * m_zoom + m_offset;
}

void CanvasWidget::wheelEvent(QWheelEvent* e) {
    if (m_image.isNull()) return;
    const double factor = (e->angleDelta().y() > 0) ? 1.15 : 1.0 / 1.15;
    const QPointF imgPt = widgetToImage(e->position());
    m_zoom = qBound(0.05, m_zoom * factor, 50.0);
    m_offset = e->position() - imgPt * m_zoom;
    update();
}

void CanvasWidget::keyPressEvent(QKeyEvent* e) {
    switch (e->key()) {
        case Qt::Key_Delete:
            deleteSelected();
            break;
        case Qt::Key_Escape:
            m_drawing = false; m_stage = 0; m_current = Shape();
            update();
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            if (m_tool == Tool::Polygon && m_drawing && m_current.points.size() >= 3) {
                m_shapes.append(m_current);
                m_current = Shape(); m_drawing = false;
                emit shapesChanged();
                update();
            }
            break;
        case Qt::Key_F:
            fitToView(); update();
            break;
        default:
            QWidget::keyPressEvent(e);
    }
}

void CanvasWidget::mousePressEvent(QMouseEvent* e) {
    const QPointF imgPt = widgetToImage(e->position());
    m_lastMouseImg = imgPt;
    emit mousePositionChanged(imgPt);

    if (e->button() == Qt::MiddleButton) {
        m_panning = true; m_lastPan = e->position();
        setCursor(Qt::ClosedHandCursor);
        return;
    }

    if (m_image.isNull()) return;

    if (e->button() == Qt::LeftButton) {
        switch (m_tool) {
            case Tool::Select: {
                const int idx = hitTest(imgPt);
                m_selected = idx;
                emit selectionChanged(idx);
                if (idx >= 0) { m_movingShape = true; m_dragLastImg = imgPt; }
                break;
            }
            case Tool::Point: {
                Shape s; s.type = ShapeType::Point;
                s.label = m_label; s.color = m_color; s.points = { imgPt };
                m_shapes.append(s);
                emit shapesChanged();
                break;
            }
            case Tool::Line:
            case Tool::Rect: {
                m_current = Shape();
                m_current.type = (m_tool == Tool::Line) ? ShapeType::Line : ShapeType::Rect;
                m_current.label = m_label; m_current.color = m_color;
                m_current.points = { imgPt, imgPt };
                m_drawing = true;
                break;
            }
            case Tool::RotatedRect: {
                if (m_stage == 0) {
                    m_current = Shape();
                    m_current.type = ShapeType::RotatedRect;
                    m_current.label = m_label; m_current.color = m_color;
                    m_current.points = { imgPt, imgPt, imgPt, imgPt };
                    m_stage = 1; m_drawing = true;
                } else if (m_stage == 2) {
                    m_shapes.append(m_current);
                    m_current = Shape(); m_drawing = false; m_stage = 0;
                    emit shapesChanged();
                }
                break;
            }
            case Tool::Polygon: {
                if (!m_drawing) {
                    m_current = Shape();
                    m_current.type = ShapeType::Polygon;
                    m_current.label = m_label; m_current.color = m_color;
                    m_current.points = { imgPt };
                    m_drawing = true;
                } else {
                    m_current.points.append(imgPt);
                }
                break;
            }
            case Tool::Brush: {
                m_current = Shape();
                m_current.type = ShapeType::Brush;
                m_current.label = m_label; m_current.color = m_color;
                m_current.mask = QImage(m_image.size(), QImage::Format_ARGB32);
                m_current.mask.fill(0);
                m_drawing = true;
                paintBrush(imgPt);
                break;
            }
        }
    } else if (e->button() == Qt::RightButton) {
        if (m_tool == Tool::Polygon && m_drawing && m_current.points.size() >= 3) {
            m_shapes.append(m_current);
            m_current = Shape(); m_drawing = false;
            emit shapesChanged();
        } else if (m_tool == Tool::RotatedRect && m_drawing) {
            m_drawing = false; m_stage = 0; m_current = Shape();
        }
    }
    update();
}

void CanvasWidget::mouseMoveEvent(QMouseEvent* e) {
    const QPointF imgPt = widgetToImage(e->position());
    m_lastMouseImg = imgPt;
    emit mousePositionChanged(imgPt);

    if (m_panning) {
        m_offset += e->position() - m_lastPan;
        m_lastPan = e->position();
        update();
        return;
    }

    if (m_movingShape && (e->buttons() & Qt::LeftButton) && m_selected >= 0) {
        const QPointF d = imgPt - m_dragLastImg;
        m_dragLastImg = imgPt;
        Shape& s = m_shapes[m_selected];
        for (auto& p : s.points) p += d;
        update();
        return;
    }

    if (!m_drawing) { update(); return; }

    switch (m_tool) {
        case Tool::Line:
        case Tool::Rect:
            if (m_current.points.size() == 2) m_current.points[1] = imgPt;
            break;
        case Tool::RotatedRect:
            if (m_stage == 1 && (e->buttons() & Qt::LeftButton)) {
                m_current.points[1] = imgPt;
                m_current.points[2] = imgPt;
                m_current.points[3] = m_current.points[0];
            } else if (m_stage == 2) {
                const QPointF p0 = m_current.points[0];
                const QPointF p1 = m_current.points[1];
                const QPointF d  = p1 - p0;
                const double  len = qSqrt(d.x()*d.x() + d.y()*d.y());
                if (len < 1e-6) break;
                const QPointF n(-d.y() / len, d.x() / len);
                const QPointF v = imgPt - p0;
                const double  t = v.x() * n.x() + v.y() * n.y();
                m_current.points[2] = p1 + n * t;
                m_current.points[3] = p0 + n * t;
            }
            break;
        case Tool::Brush:
            if (e->buttons() & Qt::LeftButton) paintBrush(imgPt);
            break;
        default: break;
    }
    update();
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() == Qt::MiddleButton) {
        m_panning = false;
        unsetCursor();
        return;
    }
    if (e->button() != Qt::LeftButton) return;

    if (m_movingShape) {
        m_movingShape = false;
        emit shapesChanged();
    }

    if (!m_drawing) return;

    switch (m_tool) {
        case Tool::Line:
        case Tool::Rect:
            if (m_current.points.size() == 2 &&
                (m_current.points[0] - m_current.points[1]).manhattanLength() > 2) {
                m_shapes.append(m_current);
                emit shapesChanged();
            }
            m_current = Shape(); m_drawing = false;
            break;
        case Tool::RotatedRect:
            if (m_stage == 1) {
                if ((m_current.points[0] - m_current.points[1]).manhattanLength() > 2) {
                    m_stage = 2;
                } else {
                    m_drawing = false; m_stage = 0; m_current = Shape();
                }
            }
            break;
        case Tool::Brush:
            m_shapes.append(m_current);
            m_current = Shape(); m_drawing = false;
            emit shapesChanged();
            break;
        default: break;
    }
    update();
}

void CanvasWidget::mouseDoubleClickEvent(QMouseEvent*) {
    if (m_tool == Tool::Polygon && m_drawing && m_current.points.size() >= 3) {
        m_shapes.append(m_current);
        m_current = Shape(); m_drawing = false;
        emit shapesChanged();
        update();
    }
}

void CanvasWidget::paintBrush(const QPointF& imgPt) {
    if (m_current.mask.isNull()) return;
    QPainter p(&m_current.mask);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setBrush(QColor(255, 255, 255, 255));
    p.setPen(Qt::NoPen);
    p.drawEllipse(imgPt, m_brushSize / 2.0, m_brushSize / 2.0);
}

int CanvasWidget::hitTest(const QPointF& imgPt) const {
    const double tol = 6.0 / qMax(m_zoom, 0.01);
    for (int i = m_shapes.size() - 1; i >= 0; --i) {
        const Shape& s = m_shapes[i];
        switch (s.type) {
            case ShapeType::Point:
                if (!s.points.isEmpty() &&
                    QLineF(s.points[0], imgPt).length() < tol * 1.5)
                    return i;
                break;
            case ShapeType::Line: {
                if (s.points.size() < 2) break;
                const QPointF a = s.points[0], b = s.points[1];
                const QPointF ab = b - a, ap = imgPt - a;
                const double  d2 = ab.x()*ab.x() + ab.y()*ab.y();
                if (d2 < 1e-6) break;
                double t = (ap.x()*ab.x() + ap.y()*ab.y()) / d2;
                t = qBound(0.0, t, 1.0);
                if (QLineF(a + ab * t, imgPt).length() < tol) return i;
                break;
            }
            case ShapeType::Rect: {
                if (s.points.size() < 2) break;
                if (QRectF(s.points[0], s.points[1]).normalized().contains(imgPt))
                    return i;
                break;
            }
            case ShapeType::RotatedRect:
            case ShapeType::Polygon: {
                if (s.points.size() < 3) break;
                QPolygonF poly(s.points);
                if (poly.containsPoint(imgPt, Qt::OddEvenFill)) return i;
                break;
            }
            case ShapeType::Brush: {
                if (s.mask.isNull()) break;
                const int x = int(imgPt.x()), y = int(imgPt.y());
                if (x >= 0 && y >= 0 && x < s.mask.width() && y < s.mask.height()) {
                    if (qAlpha(s.mask.pixel(x, y)) > 0) return i;
                }
                break;
            }
        }
    }
    return -1;
}

void CanvasWidget::drawShape(QPainter& p, const Shape& s, bool selected) {
    QPen pen(s.color);
    pen.setWidthF(selected ? 3.0 : 2.0);
    pen.setCosmetic(true);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    switch (s.type) {
        case ShapeType::Point: {
            if (s.points.isEmpty()) break;
            const QPointF w = imageToWidget(s.points[0]);
            p.setBrush(s.color);
            p.drawEllipse(w, 4.0, 4.0);
            break;
        }
        case ShapeType::Line: {
            if (s.points.size() < 2) break;
            p.drawLine(imageToWidget(s.points[0]), imageToWidget(s.points[1]));
            break;
        }
        case ShapeType::Rect: {
            if (s.points.size() < 2) break;
            const QPointF a = imageToWidget(s.points[0]);
            const QPointF b = imageToWidget(s.points[1]);
            p.drawRect(QRectF(a, b).normalized());
            break;
        }
        case ShapeType::RotatedRect:
        case ShapeType::Polygon: {
            if (s.points.size() < 2) break;
            QPolygonF poly;
            for (const auto& pt : s.points) poly << imageToWidget(pt);
            if (s.type == ShapeType::Polygon && s.points.size() >= 2 && !poly.isClosed())
                p.drawPolyline(poly);
            else
                p.drawPolygon(poly);
            QColor fill = s.color; fill.setAlpha(40);
            p.setBrush(fill); p.setPen(Qt::NoPen);
            p.drawPolygon(poly);
            p.setBrush(Qt::NoBrush); p.setPen(pen);
            // draw vertices
            for (const auto& pt : poly) p.drawEllipse(pt, 3, 3);
            break;
        }
        case ShapeType::Brush: {
            if (s.mask.isNull()) break;
            QImage tinted(s.mask.size(), QImage::Format_ARGB32_Premultiplied);
            tinted.fill(0);
            QPainter tp(&tinted);
            tp.setCompositionMode(QPainter::CompositionMode_Source);
            tp.fillRect(tinted.rect(), s.color);
            tp.setCompositionMode(QPainter::CompositionMode_DestinationIn);
            tp.drawImage(0, 0, s.mask);
            tp.end();

            p.save();
            p.setOpacity(selected ? 0.7 : 0.5);
            p.translate(m_offset);
            p.scale(m_zoom, m_zoom);
            p.drawImage(0, 0, tinted);
            p.restore();
            break;
        }
    }

    if (!s.label.isEmpty() && !s.points.isEmpty() && s.type != ShapeType::Brush) {
        const QPointF lp = imageToWidget(s.points[0]) + QPointF(6, -6);
        p.setPen(Qt::black);
        p.drawText(lp + QPointF(1, 1), s.label);
        p.setPen(s.color);
        p.drawText(lp, s.label);
    }
}

void CanvasWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), palette().window());

    if (m_image.isNull()) {
        p.setPen(m_themeHint);
        QFont f = p.font();
        f.setPointSize(11);
        p.setFont(f);
        p.drawText(rect(), Qt::AlignCenter,
                   "拖放或 文件 → 打开图像 / 打开文件夹");
        return;
    }

    // Checkerboard behind image (transparency hint)
    const int tile = 12;
    const QRectF imgR(m_offset.x(), m_offset.y(),
                      m_image.width() * m_zoom, m_image.height() * m_zoom);
    p.save();
    p.setClipRect(imgR.toRect());
    for (int y = int(imgR.top()); y < imgR.bottom(); y += tile) {
        for (int x = int(imgR.left()); x < imgR.right(); x += tile) {
            const bool dark = ((x / tile) + (y / tile)) % 2;
            p.fillRect(x, y, tile, tile,
                       dark ? m_checkerDark : m_checkerLight);
        }
    }
    p.restore();

    p.save();
    p.translate(m_offset);
    p.scale(m_zoom, m_zoom);
    p.drawImage(0, 0, m_image);
    p.restore();

    for (int i = 0; i < m_shapes.size(); ++i)
        drawShape(p, m_shapes[i], i == m_selected);

    if (m_drawing) {
        Shape preview = m_current;
        if (m_tool == Tool::Polygon && !preview.points.isEmpty())
            preview.points.append(m_lastMouseImg);
        drawShape(p, preview, true);

        if (m_tool == Tool::Brush) {
            const QPointF wp = imageToWidget(m_lastMouseImg);
            const double r = m_brushSize / 2.0 * m_zoom;
            QPen pen(m_color); pen.setStyle(Qt::DashLine); pen.setCosmetic(true);
            p.setPen(pen); p.setBrush(Qt::NoBrush);
            p.drawEllipse(wp, r, r);
        }
    }
}
