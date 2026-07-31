#include "UiAssets.h"

#include <QFile>

namespace UiAssets {

QString loadStyleSheet() {
    QFile f(":/style.qss");
    if (!f.open(QIODevice::ReadOnly)) return {};
    return QString::fromUtf8(f.readAll());
}

QIcon icon(const QString& name) {
    const QString path = name.startsWith(':') ? name : ":/icons/" + name;
    return QIcon(path);
}

}  // namespace UiAssets
