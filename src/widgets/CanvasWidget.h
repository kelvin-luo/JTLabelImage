#pragma once

#include "Shape.h"

#include <QImage>
#include <QPointF>
#include <QString>
#include <QVector>
#include <QWidget>

enum class Tool {
    Select,
    Point,
    Line,
    Rect,
    RotatedRect,
    Polygon,
    Brush
};

class CanvasWidget : public QWidget {
    Q_OBJECT
public:
    explicit CanvasWidget(QWidget* parent = nullptr);

    void setImage(const QImage& img, const QString& path);
    QImage image() const { return m_image; }
    QString imagePath() const { return m_imagePath; }

    void setShapes(const QVector<Shape>& shapes);
    const QVector<Shape>& shapes() const { return m_shapes; }

    void setTool(Tool t);
    Tool tool() const { return m_tool; }

    void setCurrentLabel(const QString& label, const QColor& color);
    void setBrushSize(int s) { m_brushSize = qMax(1, s); update(); }
    int brushSize() const { return m_brushSize; }

    void clearShapes();
    void deleteSelected();
    void setSelected(int idx);
    int selected() const { return m_selected; }

    void fitToView();

signals:
    void shapesChanged();
    void selectionChanged(int idx);
    void mousePositionChanged(QPointF imgPos);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;
    void keyPressEvent(QKeyEvent*) override;

private:
    QPointF widgetToImage(const QPointF& p) const;
    QPointF imageToWidget(const QPointF& p) const;
    void drawShape(QPainter& p, const Shape& s, bool selected);
    void paintBrush(const QPointF& imgPt);
    int hitTest(const QPointF& imgPt) const;

    QImage  m_image;
    QString m_imagePath;
    QVector<Shape> m_shapes;

    Tool    m_tool = Tool::Select;
    QString m_label = "object";
    QColor  m_color{Qt::red};
    int     m_brushSize = 20;

    double  m_zoom = 1.0;
    QPointF m_offset{0, 0};

    bool    m_panning = false;
    QPointF m_lastPan;

    int     m_selected = -1;
    bool    m_movingShape = false;
    QPointF m_dragLastImg;

    Shape   m_current;
    int     m_stage = 0;
    bool    m_drawing = false;
    QPointF m_lastMouseImg;
};
