#pragma once

#include <QColor>
#include <QImage>
#include <QJsonObject>
#include <QPointF>
#include <QString>
#include <QVector>

enum class ShapeType {
    Point,
    Line,
    Rect,
    RotatedRect,
    Polygon,
    Brush
};

QString shapeTypeToString(ShapeType t);
ShapeType shapeTypeFromString(const QString& s);

struct Shape {
    ShapeType type = ShapeType::Point;
    QString label;
    QColor color{Qt::red};
    QVector<QPointF> points;   // image-space coordinates
    QImage mask;               // ARGB32 mask for Brush, image-sized

    QJsonObject toJson() const;
    static Shape fromJson(const QJsonObject& obj);
};
