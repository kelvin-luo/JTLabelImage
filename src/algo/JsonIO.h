#pragma once

#include "Shape.h"

#include <QSize>
#include <QString>
#include <QVector>

namespace JsonIO {
    bool save(const QString& jsonPath,
              const QString& imagePath,
              const QSize& imageSize,
              const QVector<Shape>& shapes);

    bool load(const QString& jsonPath,
              QString& imagePath,
              QSize& imageSize,
              QVector<Shape>& shapes);
}
