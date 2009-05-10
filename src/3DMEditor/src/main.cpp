#include <QtGui/QApplication>
#include "config.h"
#include "mainwindow.h"

QSettings settings("TA3D", "3DMEditor2");
QTranslator translator;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    translator.load( QString("i18n/3dmeditor_") + settings.value("language").toString() );

    a.installTranslator(&translator);

    MainWindow w;
    w.show();
    return a.exec();
}
