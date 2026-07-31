#include <QApplication>
#include <QStyleFactory>
#include "MainWindow.h"
#include "UiAssets.h"

#ifndef KELVINLABEL_VERSION
#  define KELVINLABEL_VERSION "0.0.0"
#endif

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setOrganizationName("Kelvin");
    QApplication::setApplicationName("JTLabelImage");
    QApplication::setApplicationVersion(KELVINLABEL_VERSION);
    QApplication::setWindowIcon(UiAssets::icon("app.png"));

    QApplication::setStyle(QStyleFactory::create("Fusion"));
    const QString qss = UiAssets::loadStyleSheet();
    if (!qss.isEmpty()) app.setStyleSheet(qss);

    MainWindow w;
    w.show();
    return app.exec();
}
