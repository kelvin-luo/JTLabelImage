#include "Shape.h"

#include <QJsonArray>

QString shapeTypeToString(ShapeType t) {
    switch (t) {
        case ShapeType::Point:       return "point";
        case ShapeType::Line:        return "line";
        case ShapeType::Rect:        return "rect";
        case ShapeType::RotatedRect: return "rotated_rect";
        case ShapeType::Polygon:     return "polygon";
        case ShapeType::Brush:       return "brush";
    }
    return "point";
}

ShapeType shapeTypeFromString(const QString& s) {
    if (s == "line")          return ShapeType::Line;
    if (s == "rect")          return ShapeType::Rect;
    if (s == "rotated_rect")  return ShapeType::RotatedRect;
    if (s == "polygon")       return ShapeType::Polygon;
    if (s == "brush")         return ShapeType::Brush;
    return ShapeType::Point;
}

QJsonObject Shape::toJson() const {
    QJsonObject o;
    o["type"]  = shapeTypeToString(type);
    o["label"] = label;
    o["color"] = color.name(QColor::HexArgb);
    QJsonArray arr;
    for (const auto& p : points) {
        QJsonArray pp;
        pp.append(p.x());
        pp.append(p.y());
        arr.append(pp);
    }
    o["points"] = arr;
    return o;
}

Shape Shape::fromJson(const QJsonObject& obj) {
    Shape s;
    s.type  = shapeTypeFromString(obj.value("type").toString());
    s.label = obj.value("label").toString();
    s.color = QColor(obj.value("color").toString("#ffff0000"));
    for (const auto& v : obj.value("points").toArray()) {
        const QJsonArray pp = v.toArray();
        if (pp.size() >= 2)
            s.points.append(QPointF(pp[0].toDouble(), pp[1].toDouble()));
    }
    return s;
}
