#include "mainwindow.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QIcon icon;
    icon.addFile(":/icons/icon_16x16.png", QSize(16, 16));
    icon.addFile(":/icons/icon_32x32.png", QSize(32, 32));
    icon.addFile(":/icons/icon_48x48.png", QSize(48, 48));
    icon.addFile(":/icons/icon_64x64.png", QSize(64, 64));
    icon.addFile(":/icons/icon_128x128.png", QSize(128, 128));
    icon.addFile(":/icons/icon_256x256.png", QSize(256, 256));
    icon.addFile("/icons/icon_512x512.png", QSize(512, 512));
    icon.addFile("/icons/icon_1024x1024.png", QSize(1024,1024));
    a.setWindowIcon(icon);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "Register_App_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
    w.show();
    return a.exec();
}
