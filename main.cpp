#include "mainwindow.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QIcon icon;
    icon.addFile(":/icons/app_icon_16.png", QSize(16, 16));
    icon.addFile(":/icons/app_icon_32.png", QSize(32, 32));
    icon.addFile(":/icons/app_icon_48.png", QSize(48, 48));
    icon.addFile(":/icons/app_icon_64.png", QSize(64, 64));
    icon.addFile(":/icons/app_icon_128.png", QSize(128, 128));
    icon.addFile(":/icons/app_icon_256.png", QSize(256, 256));
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
