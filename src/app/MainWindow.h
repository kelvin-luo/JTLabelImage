#pragma once

#include "AppConfig.h"
#include "CanvasWidget.h"
#include "StepTimer.h"

#include <QColor>
#include <QMainWindow>
#include <QString>
#include <QStringList>

class AutoUpdater;
class LogPanel;
class QActionGroup;
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QSpinBox;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void openImage();
    void openFolder();
    void saveJson();
    void loadJson();
    void loadAppConfig();
    void saveAppConfig();
    void saveAppConfigAs();
    void applyConfigFromUi();
    void syncUiFromConfig();
    void checkForUpdates();
    void showAbout();
    void onShapesChanged();
    void onSelectionChanged(int idx);
    void onMouseImagePos(QPointF p);
    void onFileSelected(int row);
    void onLabelChanged();
    void onPickColor();
    void onThemeChanged();
    void applyTheme(const QString& themeId);

private:
    void setupUi();
    void setupMenus();
    void setupToolbar();
    void setupDocks();
    void loadImageFile(const QString& path);
    QString runtimeRoot() const;
    QString resolveRuntimePath(const QString& relativeOrAbsolute) const;
    QString defaultConfigPath() const;
    void log(const QString& msg);
    void ensureRuntimeDirs();
    void syncThemeMenu(const QString& themeId);

    CanvasWidget* m_canvas{nullptr};
    LogPanel*     m_log{nullptr};
    AutoUpdater*  m_updater{nullptr};

    QListWidget*  m_fileList{nullptr};
    QListWidget*  m_shapeList{nullptr};
    QComboBox*    m_labelCombo{nullptr};
    QSpinBox*     m_brushSpin{nullptr};
    QLabel*       m_statusPos{nullptr};
    QLabel*       m_colorSwatch{nullptr};

    QLineEdit*    m_inputDirEdit{nullptr};
    QLineEdit*    m_outputDirEdit{nullptr};
    QLineEdit*    m_modelsDirEdit{nullptr};
    QLineEdit*    m_updateUrlEdit{nullptr};
    QCheckBox*    m_checkUpdateBox{nullptr};
    QComboBox*    m_themeCombo{nullptr};
    QActionGroup* m_themeActionGroup{nullptr};

    QStringList   m_files;
    QString       m_currentDir;
    QString       m_configPath;
    AppConfig     m_config;
    StepTimer     m_timer;
    QColor        m_labelColor{Qt::red};
};
