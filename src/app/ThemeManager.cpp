#include "ThemeManager.h"

namespace {

ThemeColors makeTheme(const QColor& windowBg,
                      const QColor& panelBg,
                      const QColor& surfaceBg,
                      const QColor& border,
                      const QColor& text,
                      const QColor& textMuted,
                      const QColor& accent,
                      const QColor& canvasBg,
                      const QColor& canvasHint,
                      const QColor& checkerDark,
                      const QColor& checkerLight) {
    ThemeColors c;
    c.windowBg = windowBg;
    c.panelBg = panelBg;
    c.surfaceBg = surfaceBg;
    c.border = border;
    c.text = text;
    c.textMuted = textMuted;
    c.accent = accent;
    c.accentHover = accent.lighter(115);
    c.canvasBg = canvasBg;
    c.canvasHint = canvasHint;
    c.checkerDark = checkerDark;
    c.checkerLight = checkerLight;
    return c;
}

QString hex(const QColor& c) {
    return c.name(QColor::HexRgb);
}

QString accentAlpha(const QColor& c, int alpha) {
    QColor a = c;
    a.setAlpha(alpha);
    return a.name(QColor::HexArgb);
}

}  // namespace

namespace ThemeManager {

QVector<ThemeInfo> allThemes() {
    return {
        {"default",      QStringLiteral("默认（黑白）")},
        {"light_green",  QStringLiteral("浅绿")},
        {"light_blue",   QStringLiteral("浅蓝")},
        {"deep_blue",    QStringLiteral("深蓝")},
        {"dark",         QStringLiteral("暗黑")},
        {"light_yellow", QStringLiteral("浅黄")},
        {"orange",       QStringLiteral("橙色")},
    };
}

QStringList displayNames() {
    QStringList names;
    for (const auto& t : allThemes())
        names << t.displayName;
    return names;
}

QString idFromDisplayName(const QString& name) {
    for (const auto& t : allThemes()) {
        if (t.displayName == name) return t.id;
    }
    return QStringLiteral("default");
}

QString displayNameFromId(const QString& id) {
    for (const auto& t : allThemes()) {
        if (t.id == id) return t.displayName;
    }
    return QStringLiteral("默认（黑白）");
}

bool isValidId(const QString& id) {
    for (const auto& t : allThemes()) {
        if (t.id == id) return true;
    }
    return false;
}

ThemeColors colorsFor(const QString& id) {
    if (id == "light_green") {
        return makeTheme(
            QColor("#eef7f0"), QColor("#dff0e4"), QColor("#ffffff"),
            QColor("#a8cbb4"), QColor("#1f3b2a"), QColor("#4d6f5a"),
            QColor("#3aad68"),
            QColor("#e4f3e9"), QColor("#5a7d68"),
            QColor("#cfe6d6"), QColor("#eaf6ee"));
    }
    if (id == "light_blue") {
        return makeTheme(
            QColor("#eef4fb"), QColor("#dde9f7"), QColor("#ffffff"),
            QColor("#a7c0de"), QColor("#1c2f45"), QColor("#4d6480"),
            QColor("#3a7bd5"),
            QColor("#e6effa"), QColor("#5a7390"),
            QColor("#cddff2"), QColor("#ebf3fc"));
    }
    if (id == "deep_blue") {
        return makeTheme(
            QColor("#0f1a2b"), QColor("#162338"), QColor("#1b2c45"),
            QColor("#2f4566"), QColor("#e6eef9"), QColor("#9aadc8"),
            QColor("#4a9eff"),
            QColor("#0c1524"), QColor("#8aa0bf"),
            QColor("#142033"), QColor("#1a2a40"));
    }
    if (id == "dark") {
        return makeTheme(
            QColor("#2b2d33"), QColor("#32353d"), QColor("#25272c"),
            QColor("#454955"), QColor("#e8eaed"), QColor("#a8adb8"),
            QColor("#4a9eff"),
            QColor("#1e2024"), QColor("#787e8a"),
            QColor("#2a2c32"), QColor("#343740"));
    }
    if (id == "light_yellow") {
        return makeTheme(
            QColor("#fbf6e8"), QColor("#f3e9cf"), QColor("#fffdf6"),
            QColor("#d6c79a"), QColor("#3d3420"), QColor("#6f6345"),
            QColor("#c9a227"),
            QColor("#f7efd8"), QColor("#7a6d4c"),
            QColor("#ebe0c0"), QColor("#faf4e4"));
    }
    if (id == "orange") {
        return makeTheme(
            QColor("#fff3ea"), QColor("#ffe4d1"), QColor("#ffffff"),
            QColor("#e0b08a"), QColor("#3b2414"), QColor("#7a5238"),
            QColor("#e67a2e"),
            QColor("#ffecdd"), QColor("#8a5d40"),
            QColor("#f5d5ba"), QColor("#fff0e4"));
    }
    return makeTheme(
        QColor("#f5f5f5"), QColor("#e8e8e8"), QColor("#ffffff"),
        QColor("#bdbdbd"), QColor("#1a1a1a"), QColor("#666666"),
        QColor("#424242"),
        QColor("#ececec"), QColor("#777777"),
        QColor("#d8d8d8"), QColor("#f0f0f0"));
}

QString styleSheetFor(const QString& id) {
    const ThemeColors c = colorsFor(id);
    QString qss = QStringLiteral(R"(
QMainWindow, QWidget {
    background-color: @WIN@;
    color: @TEXT@;
    font-family: "Segoe UI", "Microsoft YaHei UI", sans-serif;
    font-size: 13px;
}
QMenuBar {
    background-color: @PANEL@;
    border-bottom: 1px solid @BORDER@;
    padding: 2px 4px;
}
QMenuBar::item {
    padding: 6px 12px;
    border-radius: 4px;
}
QMenuBar::item:selected {
    background-color: @ACCENT33@;
}
QMenu {
    background-color: @PANEL@;
    border: 1px solid @BORDER@;
    padding: 4px;
}
QMenu::item {
    padding: 8px 28px 8px 12px;
    border-radius: 4px;
}
QMenu::item:selected {
    background-color: @ACCENT@;
    color: #ffffff;
}
QMenu::separator {
    height: 1px;
    background: @BORDER@;
    margin: 4px 8px;
}
QToolBar {
    background-color: @PANEL@;
    border: none;
    border-bottom: 1px solid @BORDER@;
    spacing: 6px;
    padding: 6px 8px;
}
QToolBar::separator {
    background: @BORDER@;
    width: 1px;
    margin: 4px 8px;
}
QToolButton {
    background-color: transparent;
    border: 1px solid transparent;
    border-radius: 6px;
    padding: 6px;
    min-width: 28px;
    min-height: 28px;
}
QToolButton:hover {
    background-color: @SURFACE@;
    border-color: @BORDER@;
}
QToolButton:checked {
    background-color: @ACCENT44@;
    border-color: @ACCENT@;
}
QToolButton:pressed {
    background-color: @ACCENT66@;
}
QDockWidget {
    color: @MUTED@;
    font-weight: 600;
}
QDockWidget::title {
    background: @PANEL@;
    border-bottom: 1px solid @BORDER@;
    padding: 8px 10px;
    text-align: left;
}
QListWidget, QTextEdit, QLineEdit, QComboBox, QSpinBox {
    background-color: @SURFACE@;
    border: 1px solid @BORDER@;
    border-radius: 6px;
    padding: 4px 8px;
    color: @TEXT@;
    selection-background-color: @ACCENT@;
    selection-color: #ffffff;
}
QListWidget::item {
    padding: 8px 10px;
    border-radius: 4px;
    margin: 1px 2px;
}
QListWidget::item:hover {
    background-color: @PANEL@;
}
QListWidget::item:selected {
    background-color: @ACCENT@;
    color: #ffffff;
}
QComboBox:hover, QSpinBox:hover, QLineEdit:hover {
    border-color: @ACCENT@;
}
QComboBox::drop-down {
    border: none;
    width: 22px;
}
QComboBox QAbstractItemView {
    background-color: @PANEL@;
    border: 1px solid @BORDER@;
    selection-background-color: @ACCENT@;
    color: @TEXT@;
}
QPushButton {
    background-color: @PANEL@;
    border: 1px solid @BORDER@;
    border-radius: 6px;
    padding: 6px 12px;
    color: @TEXT@;
}
QPushButton:hover {
    background-color: @SURFACE@;
    border-color: @ACCENT@;
}
QPushButton:pressed {
    background-color: @ACCENT44@;
}
QCheckBox {
    color: @TEXT@;
    spacing: 8px;
}
QLabel#colorSwatch {
    border: 2px solid @BORDER@;
    border-radius: 4px;
}
QStatusBar {
    background-color: @PANEL@;
    border-top: 1px solid @BORDER@;
    color: @MUTED@;
}
QStatusBar QLabel {
    color: @MUTED@;
    padding: 0 8px;
}
QMessageBox, QFileDialog {
    background-color: @WIN@;
}
)");
    qss.replace("@WIN@", hex(c.windowBg));
    qss.replace("@PANEL@", hex(c.panelBg));
    qss.replace("@SURFACE@", hex(c.surfaceBg));
    qss.replace("@BORDER@", hex(c.border));
    qss.replace("@TEXT@", hex(c.text));
    qss.replace("@MUTED@", hex(c.textMuted));
    qss.replace("@ACCENT@", hex(c.accent));
    qss.replace("@ACCENT33@", accentAlpha(c.accent, 0x33));
    qss.replace("@ACCENT44@", accentAlpha(c.accent, 0x44));
    qss.replace("@ACCENT66@", accentAlpha(c.accent, 0x66));
    return qss;
}

}  // namespace ThemeManager
