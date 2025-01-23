#include "ui_StartWindow.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class StartWindow;
}
QT_END_NAMESPACE

class StartWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit StartWindow(QWidget *parent = nullptr);
    ~StartWindow();

signals:
    void projectOpen(std::string path);

private:
    Ui::StartWindow *ui;

    void onNewProjClicked();
};