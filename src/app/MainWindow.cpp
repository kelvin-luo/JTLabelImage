#include "MainWindow.h"

#include "AutoUpdater.h"
#include "ImageIO.h"
#include "JsonIO.h"
#include "LogPanel.h"
#include "UiAssets.h"

#include <QActionGroup>
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QDockWidget>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QSpinBox>
#include <QStatusBar>
#include <QToolBar>
#include <QUrl>
#include <QVBoxLayout>

#ifndef KELVINLABEL_VERSION
#  define KELVINLABEL_VERSION "0.0.0"
#endif

namespace {
const QStringList kImgExts = { "*.png", "*.jpg", "*.jpeg", "*.bmp", "*.tif", "*.tiff", "*.webp" };
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(QString("JTLabelImage %1").arg(KELVINLABEL_VERSION));
    setWindowIcon(UiAssets::icon("app.png"));
    resize(1400, 900);

    m_configPath = defaultConfigPath();
    if (QFile::exists(m_configPath))
        AppConfig::loadFromFile(m_configPath, m_config);
    ensureRuntimeDirs();

    m_updater = new AutoUpdater(this);
    m_updater->setLocalVersion(KELVINLABEL_VERSION);
    m_updater->setVersionUrl(m_config.updateUrl);

    setupUi();
    setupMenus();
    setupToolbar();
    setupDocks();
    syncUiFromConfig();
    onLabelChanged();

    connect(m_updater, &AutoUpdater::logMessage, this, [this](const QString& m) { log(m); });
    connect(m_updater, &AutoUpdater::checkFailed, this, [this](const QString& r) {
        log(QStringLiteral("更新检查失败: %1").arg(r));
        m_timer.stop();
        log(m_timer.dumpSummary());
    });
    connect(m_updater, &AutoUpdater::upToDate, this, [this](const QString&) {
        m_timer.stop();
        log(m_timer.dumpSummary());
    });
    connect(m_updater, &AutoUpdater::updateAvailable, this,
            [this](const QString& latest, const QString& url, const QString& notes) {
                m_timer.stop();
                log(m_timer.dumpSummary());
                const QString msg = QStringLiteral("发现新版本 %1（当前 %2）\n\n%3\n\n是否下载？")
                                        .arg(latest, KELVINLABEL_VERSION, notes);
                if (QMessageBox::question(this, "检查更新", msg) == QMessageBox::Yes) {
                    m_timer.reset();
                    m_timer.start(QStringLiteral("下载更新包"));
                    m_updater->downloadUpdate(url);
                }
            });
    connect(m_updater, &AutoUpdater::downloadProgress, this,
            [this](qint64 received, qint64 total) {
                if (total > 0)
                    statusBar()->showMessage(
                        QStringLiteral("下载中 %1/%2").arg(received).arg(total), 1000);
            });
    connect(m_updater, &AutoUpdater::downloadFinished, this, [this](const QString& path) {
        m_timer.stop();
        log(m_timer.dumpSummary());
        const auto ret = QMessageBox::information(
            this, "下载完成",
            QStringLiteral("安装包已保存到:\n%1\n\n打开所在文件夹？").arg(path),
            QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes)
            QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(path).absolutePath()));
    });
    connect(m_updater, &AutoUpdater::downloadFailed, this, [this](const QString& r) {
        m_timer.stop();
        log(QStringLiteral("下载失败: %1").arg(r));
        log(m_timer.dumpSummary());
    });

    log(QStringLiteral("JTLabelImage %1 启动").arg(KELVINLABEL_VERSION));
    log(QStringLiteral("运行目录: %1").arg(runtimeRoot()));

    if (m_config.checkUpdateOnStartup)
        QMetaObject::invokeMethod(this, &MainWindow::checkForUpdates, Qt::QueuedConnection);
}

QString MainWindow::runtimeRoot() const {
    return QDir(QCoreApplication::applicationDirPath()).absolutePath();
}

QString MainWindow::resolveRuntimePath(const QString& relativeOrAbsolute) const {
    const QFileInfo fi(relativeOrAbsolute);
    if (fi.isAbsolute()) return QDir::cleanPath(relativeOrAbsolute);
    return QDir(runtimeRoot()).filePath(relativeOrAbsolute);
}

QString MainWindow::defaultConfigPath() const {
    return QDir(runtimeRoot()).filePath("kelvinlabel_config.json");
}

void MainWindow::ensureRuntimeDirs() {
    QDir().mkpath(resolveRuntimePath(m_config.inputDir));
    QDir().mkpath(resolveRuntimePath(m_config.outputDir));
    QDir().mkpath(resolveRuntimePath(m_config.modelsDir));
}

void MainWindow::log(const QString& msg) {
    if (m_log) m_log->append(msg);
    statusBar()->showMessage(msg, 4000);
}

void MainWindow::setupUi() {
    m_canvas = new CanvasWidget(this);
    setCentralWidget(m_canvas);
    connect(m_canvas, &CanvasWidget::shapesChanged, this, &MainWindow::onShapesChanged);
    connect(m_canvas, &CanvasWidget::selectionChanged, this, &MainWindow::onSelectionChanged);
    connect(m_canvas, &CanvasWidget::mousePositionChanged, this, &MainWindow::onMouseImagePos);

    m_statusPos = new QLabel(this);
    statusBar()->showMessage("就绪");
    statusBar()->addPermanentWidget(m_statusPos);
}

void MainWindow::setupMenus() {
    auto menuAct = [](QMenu* menu, const QString& text, const QIcon& icon,
                      const QKeySequence& sc) {
        QAction* a = menu->addAction(icon, text);
        if (!sc.isEmpty()) a->setShortcut(sc);
        return a;
    };

    QMenu* file = menuBar()->addMenu("文件(&F)");
    connect(menuAct(file, "打开图像...", UiAssets::icon("open_image.png"),
                    QKeySequence::Open),
            &QAction::triggered, this, &MainWindow::openImage);
    connect(menuAct(file, "打开文件夹...", UiAssets::icon("open_folder.png"), {}),
            &QAction::triggered, this, &MainWindow::openFolder);
    file->addSeparator();
    connect(menuAct(file, "保存标注", UiAssets::icon("save.png"), QKeySequence::Save),
            &QAction::triggered, this, &MainWindow::saveJson);
    connect(menuAct(file, "加载标注...", UiAssets::icon("load.png"), {}),
            &QAction::triggered, this, &MainWindow::loadJson);
    file->addSeparator();
    connect(menuAct(file, "加载程序配置", UiAssets::icon("load.png"), {}),
            &QAction::triggered, this, &MainWindow::loadAppConfig);
    connect(menuAct(file, "保存程序配置", UiAssets::icon("save.png"), {}),
            &QAction::triggered, this, &MainWindow::saveAppConfig);
    connect(menuAct(file, "另存程序配置", UiAssets::icon("save.png"), {}),
            &QAction::triggered, this, &MainWindow::saveAppConfigAs);
    file->addSeparator();
    connect(menuAct(file, "退出", QIcon(), QKeySequence("Ctrl+Q")),
            &QAction::triggered, this, &QWidget::close);

    QMenu* edit = menuBar()->addMenu("编辑(&E)");
    connect(menuAct(edit, "删除所选", UiAssets::icon("delete.png"),
                    QKeySequence::Delete),
            &QAction::triggered, m_canvas, &CanvasWidget::deleteSelected);
    connect(menuAct(edit, "清空全部", UiAssets::icon("clear.png"), {}),
            &QAction::triggered, this, [this] { m_canvas->clearShapes(); });

    QMenu* view = menuBar()->addMenu("视图(&V)");
    connect(menuAct(view, "适应窗口", UiAssets::icon("fit.png"), QKeySequence("F")),
            &QAction::triggered, this, [this] {
                m_canvas->fitToView();
                m_canvas->update();
            });

    QMenu* help = menuBar()->addMenu("帮助(&H)");
    connect(menuAct(help, "检查更新...", QIcon(), {}),
            &QAction::triggered, this, &MainWindow::checkForUpdates);
    connect(menuAct(help, "关于...", QIcon(), {}),
            &QAction::triggered, this, &MainWindow::showAbout);
}

void MainWindow::setupToolbar() {
    QToolBar* tb = addToolBar("工具");
    tb->setObjectName("mainToolBar");
    tb->setMovable(false);
    tb->setIconSize(QSize(22, 22));
    tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    QActionGroup* group = new QActionGroup(this);
    group->setExclusive(true);

    auto addTool = [&](const QString& text, Tool t, const QString& shortcut,
                       const QString& iconFile) {
        QAction* a = tb->addAction(UiAssets::icon(iconFile), text);
        a->setCheckable(true);
        a->setShortcut(QKeySequence(shortcut));
        group->addAction(a);
        connect(a, &QAction::triggered, this, [this, t] { m_canvas->setTool(t); });
        return a;
    };
    addTool("选择", Tool::Select, "S", "tool_select.png")->setChecked(true);
    addTool("点", Tool::Point, "P", "tool_point.png");
    addTool("直线", Tool::Line, "L", "tool_line.png");
    addTool("矩形", Tool::Rect, "R", "tool_rect.png");
    addTool("旋转矩形", Tool::RotatedRect, "T", "tool_rotrect.png");
    addTool("多边形", Tool::Polygon, "G", "tool_polygon.png");
    addTool("涂抹", Tool::Brush, "B", "tool_brush.png");

    tb->addSeparator();
    tb->addWidget(new QLabel(" 标签 "));
    m_labelCombo = new QComboBox();
    m_labelCombo->setEditable(true);
    m_labelCombo->addItems({ "object", "background", "defect" });
    m_labelCombo->setMinimumWidth(120);
    tb->addWidget(m_labelCombo);
    connect(m_labelCombo, &QComboBox::currentTextChanged,
            this, &MainWindow::onLabelChanged);

    m_colorSwatch = new QLabel();
    m_colorSwatch->setObjectName("colorSwatch");
    m_colorSwatch->setFixedSize(22, 22);
    m_colorSwatch->setToolTip("当前标签颜色");
    tb->addWidget(m_colorSwatch);

    QAction* colorA = tb->addAction(UiAssets::icon("color.png"), "颜色");
    colorA->setToolTip("选择标签颜色");
    connect(colorA, &QAction::triggered, this, &MainWindow::onPickColor);

    tb->addSeparator();
    tb->addWidget(new QLabel(" 笔刷 "));
    m_brushSpin = new QSpinBox();
    m_brushSpin->setRange(1, 500);
    m_brushSpin->setValue(20);
    m_brushSpin->setFixedWidth(64);
    tb->addWidget(m_brushSpin);
    connect(m_brushSpin, qOverload<int>(&QSpinBox::valueChanged),
            m_canvas, &CanvasWidget::setBrushSize);

    tb->addSeparator();
    QAction* fitA = tb->addAction(UiAssets::icon("fit.png"), "适应");
    fitA->setShortcut(QKeySequence("F"));
    fitA->setToolTip("适应窗口 (F)");
    connect(fitA, &QAction::triggered, this, [this] {
        m_canvas->fitToView();
        m_canvas->update();
    });
}

void MainWindow::setupDocks() {
    QDockWidget* fdock = new QDockWidget("图像列表", this);
    m_fileList = new QListWidget();
    m_fileList->setAlternatingRowColors(true);
    fdock->setWidget(m_fileList);
    addDockWidget(Qt::LeftDockWidgetArea, fdock);
    connect(m_fileList, &QListWidget::currentRowChanged,
            this, &MainWindow::onFileSelected);

    QDockWidget* sdock = new QDockWidget("标注列表", this);
    m_shapeList = new QListWidget();
    m_shapeList->setAlternatingRowColors(true);
    sdock->setWidget(m_shapeList);
    addDockWidget(Qt::RightDockWidgetArea, sdock);
    connect(m_shapeList, &QListWidget::currentRowChanged,
            this, [this](int row) { m_canvas->setSelected(row); });

    QDockWidget* pdock = new QDockWidget("程序参数", this);
    QWidget* panel = new QWidget();
    auto* form = new QFormLayout(panel);
    m_inputDirEdit = new QLineEdit();
    m_outputDirEdit = new QLineEdit();
    m_modelsDirEdit = new QLineEdit();
    m_updateUrlEdit = new QLineEdit();
    m_checkUpdateBox = new QCheckBox("启动时检查更新");
    form->addRow("输入目录", m_inputDirEdit);
    form->addRow("输出目录", m_outputDirEdit);
    form->addRow("模型目录", m_modelsDirEdit);
    form->addRow("更新 URL", m_updateUrlEdit);
    form->addRow("", m_checkUpdateBox);

    auto* btnRow = new QHBoxLayout();
    auto* applyBtn = new QPushButton("应用参数");
    auto* loadBtn = new QPushButton("加载配置");
    auto* saveBtn = new QPushButton("保存配置");
    auto* saveAsBtn = new QPushButton("另存配置");
    btnRow->addWidget(applyBtn);
    btnRow->addWidget(loadBtn);
    btnRow->addWidget(saveBtn);
    btnRow->addWidget(saveAsBtn);
    form->addRow(btnRow);
    connect(applyBtn, &QPushButton::clicked, this, &MainWindow::applyConfigFromUi);
    connect(loadBtn, &QPushButton::clicked, this, &MainWindow::loadAppConfig);
    connect(saveBtn, &QPushButton::clicked, this, &MainWindow::saveAppConfig);
    connect(saveAsBtn, &QPushButton::clicked, this, &MainWindow::saveAppConfigAs);

    pdock->setWidget(panel);
    addDockWidget(Qt::RightDockWidgetArea, pdock);

    QDockWidget* ldock = new QDockWidget("日志", this);
    m_log = new LogPanel();
    ldock->setWidget(m_log);
    addDockWidget(Qt::BottomDockWidgetArea, ldock);
}

void MainWindow::syncUiFromConfig() {
    if (m_inputDirEdit) m_inputDirEdit->setText(m_config.inputDir);
    if (m_outputDirEdit) m_outputDirEdit->setText(m_config.outputDir);
    if (m_modelsDirEdit) m_modelsDirEdit->setText(m_config.modelsDir);
    if (m_updateUrlEdit) m_updateUrlEdit->setText(m_config.updateUrl);
    if (m_checkUpdateBox) m_checkUpdateBox->setChecked(m_config.checkUpdateOnStartup);
    if (m_labelCombo) {
        const int idx = m_labelCombo->findText(m_config.defaultLabel);
        if (idx >= 0) m_labelCombo->setCurrentIndex(idx);
        else m_labelCombo->setEditText(m_config.defaultLabel);
    }
    m_labelColor = m_config.labelColor;
    if (m_brushSpin) m_brushSpin->setValue(m_config.brushSize);
    m_updater->setVersionUrl(m_config.updateUrl);
    onLabelChanged();
}

void MainWindow::applyConfigFromUi() {
    m_config.inputDir = m_inputDirEdit->text().trimmed();
    m_config.outputDir = m_outputDirEdit->text().trimmed();
    m_config.modelsDir = m_modelsDirEdit->text().trimmed();
    m_config.updateUrl = m_updateUrlEdit->text().trimmed();
    m_config.checkUpdateOnStartup = m_checkUpdateBox->isChecked();
    m_config.defaultLabel = m_labelCombo->currentText();
    m_config.labelColor = m_labelColor;
    m_config.brushSize = m_brushSpin->value();
    m_updater->setVersionUrl(m_config.updateUrl);
    ensureRuntimeDirs();
    onLabelChanged();
    log(QStringLiteral("已应用界面参数"));
}

void MainWindow::openImage() {
    const QString start = m_config.lastOpenDir.isEmpty()
        ? resolveRuntimePath(m_config.inputDir)
        : m_config.lastOpenDir;
    const QString path = QFileDialog::getOpenFileName(
        this, "打开图像", start,
        "图像 (*.png *.jpg *.jpeg *.bmp *.tif *.tiff *.webp)");
    if (path.isEmpty()) return;
    m_config.lastOpenDir = QFileInfo(path).absolutePath();
    loadImageFile(path);
}

void MainWindow::openFolder() {
    const QString start = m_config.lastOpenDir.isEmpty()
        ? resolveRuntimePath(m_config.inputDir)
        : m_config.lastOpenDir;
    const QString dir = QFileDialog::getExistingDirectory(this, "打开文件夹", start);
    if (dir.isEmpty()) return;
    m_currentDir = dir;
    m_config.lastOpenDir = dir;
    QDir d(dir);
    m_files.clear();
    m_fileList->clear();
    for (const QString& name : d.entryList(kImgExts, QDir::Files, QDir::Name)) {
        m_files << d.absoluteFilePath(name);
        m_fileList->addItem(name);
    }
    log(QStringLiteral("打开文件夹: %1 (%2 张)").arg(dir).arg(m_files.size()));
    if (!m_files.isEmpty()) m_fileList->setCurrentRow(0);
}

void MainWindow::onFileSelected(int row) {
    if (row < 0 || row >= m_files.size()) return;
    loadImageFile(m_files[row]);
}

void MainWindow::loadImageFile(const QString& path) {
    m_timer.reset();
    m_timer.start(QStringLiteral("加载图像"));
    const QImage img = ImageIO::load(path);
    m_timer.stop();
    if (img.isNull()) {
        log(QStringLiteral("无法加载图像: %1").arg(path));
        QMessageBox::warning(this, "错误", "无法加载图像: " + path);
        log(m_timer.dumpSummary());
        return;
    }
    m_canvas->setImage(img, path);
    setWindowTitle(QString("JTLabelImage %1 - %2")
                       .arg(KELVINLABEL_VERSION, QFileInfo(path).fileName()));

    const QString outDir = resolveRuntimePath(m_config.outputDir);
    const QString jpath = QDir(outDir).filePath(
        QFileInfo(path).completeBaseName() + ".json");
    if (QFile::exists(jpath)) {
        m_timer.start(QStringLiteral("自动加载标注"));
        QString ip; QSize sz; QVector<Shape> shapes;
        if (JsonIO::load(jpath, ip, sz, shapes))
            m_canvas->setShapes(shapes);
        m_timer.stop();
    }
    onShapesChanged();
    log(QStringLiteral("已加载: %1 (%2x%3)")
            .arg(path).arg(img.width()).arg(img.height()));
    log(m_timer.dumpSummary());
}

void MainWindow::saveJson() {
    if (m_canvas->image().isNull()) {
        QMessageBox::information(this, "提示", "请先加载图像");
        return;
    }
    m_timer.reset();
    m_timer.start(QStringLiteral("保存标注"));
    const QString outDir = resolveRuntimePath(m_config.outputDir);
    QDir().mkpath(outDir);
    const QString base  = QFileInfo(m_canvas->imagePath()).completeBaseName();
    const QString jpath = QDir(outDir).filePath(base + ".json");
    const bool ok = JsonIO::save(jpath, m_canvas->imagePath(),
                                 m_canvas->image().size(), m_canvas->shapes());
    m_timer.stop();
    if (ok) {
        log(QStringLiteral("已保存标注: %1").arg(jpath));
    } else {
        log(QStringLiteral("保存标注失败: %1").arg(jpath));
        QMessageBox::warning(this, "错误", "保存失败");
    }
    log(m_timer.dumpSummary());
}

void MainWindow::loadJson() {
    const QString outDir = resolveRuntimePath(m_config.outputDir);
    const QString jpath = QFileDialog::getOpenFileName(
        this, "加载标注", outDir, "JSON (*.json)");
    if (jpath.isEmpty()) return;
    m_timer.reset();
    m_timer.start(QStringLiteral("加载标注"));
    QString ip; QSize sz; QVector<Shape> shapes;
    if (!JsonIO::load(jpath, ip, sz, shapes)) {
        m_timer.stop();
        log(QStringLiteral("JSON 解析失败: %1").arg(jpath));
        QMessageBox::warning(this, "错误", "JSON 解析失败");
        log(m_timer.dumpSummary());
        return;
    }
    m_timer.stop();
    if (QFile::exists(ip)) loadImageFile(ip);
    m_canvas->setShapes(shapes);
    log(QStringLiteral("已加载标注: %1 (%2 个形状)").arg(jpath).arg(shapes.size()));
    log(m_timer.dumpSummary());
}

void MainWindow::loadAppConfig() {
    const QString path = QFileDialog::getOpenFileName(
        this, "加载程序配置", m_configPath, "JSON (*.json)");
    if (path.isEmpty()) return;
    m_timer.reset();
    m_timer.start(QStringLiteral("加载程序配置"));
    AppConfig cfg;
    const bool ok = AppConfig::loadFromFile(path, cfg);
    m_timer.stop();
    if (!ok) {
        log(QStringLiteral("加载配置失败: %1").arg(path));
        QMessageBox::warning(this, "错误", "配置解析失败");
        log(m_timer.dumpSummary());
        return;
    }
    m_config = cfg;
    m_configPath = path;
    syncUiFromConfig();
    ensureRuntimeDirs();
    log(QStringLiteral("已加载程序配置: %1").arg(path));
    log(m_timer.dumpSummary());
}

void MainWindow::saveAppConfig() {
    applyConfigFromUi();
    m_timer.reset();
    m_timer.start(QStringLiteral("保存程序配置"));
    if (m_configPath.isEmpty()) m_configPath = defaultConfigPath();
    const bool ok = m_config.saveToFile(m_configPath);
    m_timer.stop();
    if (ok) log(QStringLiteral("已保存程序配置: %1").arg(m_configPath));
    else {
        log(QStringLiteral("保存配置失败: %1").arg(m_configPath));
        QMessageBox::warning(this, "错误", "保存配置失败");
    }
    log(m_timer.dumpSummary());
}

void MainWindow::saveAppConfigAs() {
    applyConfigFromUi();
    const QString path = QFileDialog::getSaveFileName(
        this, "另存程序配置", m_configPath, "JSON (*.json)");
    if (path.isEmpty()) return;
    m_timer.reset();
    m_timer.start(QStringLiteral("另存程序配置"));
    const bool ok = m_config.saveToFile(path);
    m_timer.stop();
    if (ok) {
        m_configPath = path;
        log(QStringLiteral("已另存程序配置: %1").arg(path));
    } else {
        log(QStringLiteral("另存配置失败: %1").arg(path));
        QMessageBox::warning(this, "错误", "另存配置失败");
    }
    log(m_timer.dumpSummary());
}

void MainWindow::checkForUpdates() {
    applyConfigFromUi();
    m_timer.reset();
    m_timer.start(QStringLiteral("检查更新"));
    m_updater->checkForUpdates();
}

void MainWindow::showAbout() {
    QMessageBox::about(
        this, "关于 JTLabelImage",
        QStringLiteral(
            "<h3>JTLabelImage</h3>"
            "<p>版本: <b>%1</b></p>"
            "<p>基于 Qt6 / OpenCV / CMake 的图像标注工具</p>"
            "<p>架构: 算法 + Qt 控件 + App</p>")
            .arg(KELVINLABEL_VERSION));
}

void MainWindow::onShapesChanged() {
    m_shapeList->blockSignals(true);
    m_shapeList->clear();
    for (const auto& s : m_canvas->shapes()) {
        m_shapeList->addItem(QString("[%1] %2")
            .arg(shapeTypeToString(s.type), s.label));
    }
    m_shapeList->blockSignals(false);
    if (m_canvas->selected() >= 0)
        m_shapeList->setCurrentRow(m_canvas->selected());
}

void MainWindow::onSelectionChanged(int idx) {
    m_shapeList->blockSignals(true);
    m_shapeList->setCurrentRow(idx);
    m_shapeList->blockSignals(false);
}

void MainWindow::onMouseImagePos(QPointF p) {
    m_statusPos->setText(QString("x: %1  y: %2")
        .arg(p.x(), 0, 'f', 1).arg(p.y(), 0, 'f', 1));
}

void MainWindow::onLabelChanged() {
    if (!m_labelCombo) return;
    m_canvas->setCurrentLabel(m_labelCombo->currentText(), m_labelColor);
    if (m_colorSwatch) {
        QPixmap pm(m_colorSwatch->size());
        pm.fill(m_labelColor);
        m_colorSwatch->setPixmap(pm);
    }
}

void MainWindow::onPickColor() {
    const QColor c = QColorDialog::getColor(m_labelColor, this, "选择标签颜色");
    if (!c.isValid()) return;
    m_labelColor = c;
    onLabelChanged();
}
