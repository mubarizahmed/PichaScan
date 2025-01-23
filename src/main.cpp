#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    // qputenv("QT_DEBUG_PLUGINS", QByteArray("1"));
    QApplication app(argc, argv);

    MainWindow w;

    return app.exec();
}
