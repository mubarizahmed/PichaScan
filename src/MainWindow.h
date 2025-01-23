#pragma once

#include "ScannerInterface.h"
#include "ui_MainWindow.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class ImageEditorView; // Forward declaration
class CroppedView;     // Forward declaration

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
public slots:
    void handleMapMarker(double latitude, double longitude);

signals:
    void setInitialCoordinates(double latitude, double longitude);

private slots:

    void onScanButtonClicked();
    void onSaveButtonClicked();
    void onFindScannerButtonClicked();
    void onScannerSelectionChanged(QString scannerName);
    void onColorOptionChanged(int index);
    void onDpiOptionChanged(int index);
    void updateThumbnailsList(std::vector<std::vector<cv::Point>> quads);

private:
    Ui::MainWindow *ui;

    std::unique_ptr<ScannerInterface> scanner;
    ImageEditorView *scanView;
    QGraphicsScene *scanScene;
    CroppedView *croppedView;

    cv::Mat scanImage;
    int scanOrientation;
    std::vector<cv::Mat> croppedImages;
    std::vector<int> croppedOrientation;

    QDateTime imageDateTime;
    std::pair<double, double> imageLocation;

    static void displayMatInGraphicsView(const cv::Mat &mat, ImageEditorView *graphicsView, QGraphicsScene *scene);
    static QImage matToQImage(const cv::Mat &mat);

    static cv::Point2f computeCentroid(const std::vector<cv::Point>& quad);
    static void sortQuadsByCenter(std::vector<std::vector<cv::Point>>& quads, const cv::Point& reference = cv::Point(0, 0));
};
