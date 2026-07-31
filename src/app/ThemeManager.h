#pragma once

#include <QColor>
#include <QString>
#include <QStringList>
#include <QVector>

struct ThemeColors {
    QColor windowBg;
    QColor panelBg;
    QColor surfaceBg;
    QColor border;
    QColor text;
    QColor textMuted;
    QColor accent;
    QColor accentHover;
    QColor canvasBg;
    QColor canvasHint;
    QColor checkerDark;
    QColor checkerLight;
};

struct ThemeInfo {
    QString id;
    QString displayName;
};

namespace ThemeManager {

QVector<ThemeInfo> allThemes();
QStringList displayNames();
QString idFromDisplayName(const QString& name);
QString displayNameFromId(const QString& id);
bool isValidId(const QString& id);

ThemeColors colorsFor(const QString& id);
QString styleSheetFor(const QString& id);

}  // namespace ThemeManager
