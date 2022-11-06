#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QSurfaceFormat>

int main(int argc, char *argv[])
{
    // create the QT application
    QApplication a(argc, argv);

    // add localization
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "PaperariumDesign_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    // show the main window
    MainWindow w;
    w.show();
    w.raise();

    // execute the application
    return a.exec();
}
