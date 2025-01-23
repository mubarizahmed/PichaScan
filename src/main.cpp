#include "MainWindow.h"
#include "StartWindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
    // qputenv("QT_DEBUG_PLUGINS", QByteArray("1"));
    QApplication app(argc, argv);

    StartWindow *s = new StartWindow();

    QObject::connect(s, &StartWindow::projectOpen, [=](const std::string &projectPath) {
        qDebug() << projectPath.c_str();
        MainWindow *mainWindow = new MainWindow(projectPath);
        mainWindow->show();
        s->deleteLater(); // Schedule deletion after the main window is shown
    });

    s->show();

    return app.exec();
}
