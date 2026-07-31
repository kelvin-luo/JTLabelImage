#pragma once

#include <QColor>
#include <QJsonObject>
#include <QString>

struct AppConfig {
    QString inputDir{"input"};
    QString outputDir{"output"};
    QString modelsDir{"models"};
    QString defaultLabel{"object"};
    QColor  labelColor{Qt::red};
    int     brushSize{20};
    QString updateUrl{"https://raw.githubusercontent.com/kelvin-luo/kelvin-luo.github.io/master/version.json"};
    bool    checkUpdateOnStartup{true};
    QString lastOpenDir;
    QString themeId{"default"};

    QJsonObject toJson() const;
    static AppConfig fromJson(const QJsonObject& obj);

    bool saveToFile(const QString& path) const;
    static bool loadFromFile(const QString& path, AppConfig& out);
};
