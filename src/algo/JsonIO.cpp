#include "JsonIO.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {

QImage maskToGrayscale(const QImage& mask) {
    QImage gray(mask.size(), QImage::Format_Grayscale8);
    const QImage src = mask.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < src.height(); ++y) {
        const QRgb* sp = reinterpret_cast<const QRgb*>(src.constScanLine(y));
        uchar* dp = gray.scanLine(y);
        for (int x = 0; x < src.width(); ++x)
            dp[x] = static_cast<uchar>(qAlpha(sp[x]));
    }
    return gray;
}

QImage grayscaleToMask(const QImage& gray) {
    const QImage g = gray.convertToFormat(QImage::Format_Grayscale8);
    QImage out(g.size(), QImage::Format_ARGB32);
    out.fill(0);
    for (int y = 0; y < g.height(); ++y) {
        const uchar* sp = g.constScanLine(y);
        QRgb* dp = reinterpret_cast<QRgb*>(out.scanLine(y));
        for (int x = 0; x < g.width(); ++x)
            dp[x] = qRgba(255, 255, 255, sp[x]);
    }
    return out;
}

}

bool JsonIO::save(const QString& jsonPath,
                  const QString& imagePath,
                  const QSize& imageSize,
                  const QVector<Shape>& shapes) {
    const QFileInfo jfi(jsonPath);
    const QDir dir = jfi.dir();
    const QString base = jfi.completeBaseName();

    QJsonArray shapesArr;
    int brushIdx = 0;
    for (const auto& s : shapes) {
        QJsonObject obj = s.toJson();
        if (s.type == ShapeType::Brush && !s.mask.isNull()) {
            const QString fname = QString("%1_brush_%2.png").arg(base).arg(brushIdx++);
            maskToGrayscale(s.mask).save(dir.filePath(fname), "PNG");
            obj["mask"] = fname;
            obj.remove("points");
        }
        shapesArr.append(obj);
    }

    QJsonObject root;
    root["version"]     = "0.1";
    root["imagePath"]   = dir.relativeFilePath(imagePath);
    root["imageWidth"]  = imageSize.width();
    root["imageHeight"] = imageSize.height();
    root["shapes"]      = shapesArr;

    QFile f(jsonPath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    return true;
}

bool JsonIO::load(const QString& jsonPath,
                  QString& imagePath,
                  QSize& imageSize,
                  QVector<Shape>& shapes) {
    QFile f(jsonPath);
    if (!f.open(QIODevice::ReadOnly)) return false;
    const QJsonObject root = QJsonDocument::fromJson(f.readAll()).object();

    const QDir dir = QFileInfo(jsonPath).dir();
    imagePath = dir.absoluteFilePath(root.value("imagePath").toString());
    imageSize = QSize(root.value("imageWidth").toInt(),
                      root.value("imageHeight").toInt());

    shapes.clear();
    for (const auto& v : root.value("shapes").toArray()) {
        const QJsonObject o = v.toObject();
        Shape s = Shape::fromJson(o);
        if (s.type == ShapeType::Brush) {
            const QString maskFile = o.value("mask").toString();
            if (!maskFile.isEmpty()) {
                QImage img(dir.filePath(maskFile));
                if (!img.isNull()) s.mask = grayscaleToMask(img);
            }
        }
        shapes.append(s);
    }
    return true;
}
