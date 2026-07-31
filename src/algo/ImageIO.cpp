#include "ImageIO.h"

#include <QByteArray>
#include <QFile>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <vector>

namespace ImageIO {

QImage load(const QString& path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return QImage();
    const QByteArray data = f.readAll();
    std::vector<uchar> buf(data.begin(), data.end());
    cv::Mat m = cv::imdecode(buf, cv::IMREAD_UNCHANGED);
    if (m.empty()) return QImage();

    cv::Mat conv;
    if (m.channels() == 1)      cv::cvtColor(m, conv, cv::COLOR_GRAY2RGB);
    else if (m.channels() == 3) cv::cvtColor(m, conv, cv::COLOR_BGR2RGB);
    else if (m.channels() == 4) cv::cvtColor(m, conv, cv::COLOR_BGRA2RGBA);
    else return QImage();

    const QImage::Format fmt = (conv.channels() == 4)
        ? QImage::Format_RGBA8888 : QImage::Format_RGB888;
    return QImage(conv.data, conv.cols, conv.rows, int(conv.step), fmt).copy();
}

}  // namespace ImageIO
