#pragma once

#include <QMainWindow>
#include "ScannerInterface.h"
#include "ui_MainWindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    
    void onScanButtonClicked();
    void onFindScannerButtonClicked();
    void onScannerSelectionChanged(QString scannerName);

private:
    Ui::MainWindow *ui;

    std::unique_ptr<ScannerInterface> scanner;

    static void displayMatInGraphicsView(const cv::Mat& mat, QGraphicsView* graphicsView);
    static QImage matToQImage(const cv::Mat& mat);

};
