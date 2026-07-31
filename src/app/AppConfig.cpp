#include "AppConfig.h"

#include <QFile>
#include <QJsonDocument>

QJsonObject AppConfig::toJson() const {
    QJsonObject o;
    o["inputDir"] = inputDir;
    o["outputDir"] = outputDir;
    o["modelsDir"] = modelsDir;
    o["defaultLabel"] = defaultLabel;
    o["labelColor"] = labelColor.name(QColor::HexArgb);
    o["brushSize"] = brushSize;
    o["updateUrl"] = updateUrl;
    o["checkUpdateOnStartup"] = checkUpdateOnStartup;
    o["lastOpenDir"] = lastOpenDir;
    o["themeId"] = themeId;
    return o;
}

AppConfig AppConfig::fromJson(const QJsonObject& obj) {
    AppConfig c;
    c.inputDir = obj.value("inputDir").toString(c.inputDir);
    c.outputDir = obj.value("outputDir").toString(c.outputDir);
    c.modelsDir = obj.value("modelsDir").toString(c.modelsDir);
    c.defaultLabel = obj.value("defaultLabel").toString(c.defaultLabel);
    c.labelColor = QColor(obj.value("labelColor").toString(c.labelColor.name(QColor::HexArgb)));
    c.brushSize = obj.value("brushSize").toInt(c.brushSize);
    c.updateUrl = obj.value("updateUrl").toString(c.updateUrl);
    c.checkUpdateOnStartup = obj.value("checkUpdateOnStartup").toBool(c.checkUpdateOnStartup);
    c.lastOpenDir = obj.value("lastOpenDir").toString(c.lastOpenDir);
    c.themeId = obj.value("themeId").toString(c.themeId);
    return c;
}

bool AppConfig::saveToFile(const QString& path) const {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
    f.write(QJsonDocument(toJson()).toJson(QJsonDocument::Indented));
    return true;
}

bool AppConfig::loadFromFile(const QString& path, AppConfig& out) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return false;
    const QJsonObject root = QJsonDocument::fromJson(f.readAll()).object();
    if (root.isEmpty()) return false;
    out = fromJson(root);
    return true;
}
