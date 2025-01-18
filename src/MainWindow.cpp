#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "ScanProcessor.h"
#include "ScannerInterface.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {

    scanner = nullptr;

    ui->setupUi(this);

    // Suppose you have a QPushButton named scanButton in your .ui
    connect(ui->scanButton, &QPushButton::clicked,
            this, &MainWindow::onScanButtonClicked);

    connect(ui->btnFindScanners, &QPushButton::clicked, this, &MainWindow::onFindScannerButtonClicked);

    connect(ui->comboScanners, SIGNAL(currentTextChanged(QString)), this, SLOT(onScannerSelectionChanged(QString)));

    onFindScannerButtonClicked();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onScanButtonClicked() {

    if (!scanner) {
        QMessageBox::warning(this, "Error", "No suitable scanner backend found!");
        return;
    }

    // Perform the scan (returns a large image that might contain multiple photos)
    cv::Mat scannedImage = scanner->scanImage();
    if (scannedImage.empty()) {
        QMessageBox::warning(this, "Error", "Failed to scan or no image returned.");
        return;
    }

    // Display the scanned image in the graphics view
    MainWindow::displayMatInGraphicsView(scannedImage, ui->scanView);

    // // Process the scanned image to find multiple photos
    // ScanProcessor processor;
    // std::vector<cv::Mat> croppedPhotos = processor.detectAndCropPhotos(scannedImage);

    // if (croppedPhotos.empty()) {
    //     QMessageBox::information(this, "Info", "No distinct photos detected.");
    //     return;
    // }

    // // For demonstration, let’s just save them out with a file dialog
    // // In reality, you might do more advanced UI or metadata tagging
    // QString dir = QFileDialog::getExistingDirectory(this, "Choose Save Directory");
    // if (dir.isEmpty()) {
    //     return;
    // }

    // for (size_t i = 0; i < croppedPhotos.size(); i++) {
    //     QString filename = QString("%1/photo_%2.jpg").arg(dir).arg(i);
    //     // Convert from cv::Mat to a QImage if you want to use Qt’s saving, or use OpenCV’s imwrite directly
    //     cv::imwrite(filename.toStdString(), croppedPhotos[i]);
    // }

    // QMessageBox::information(this, "Success",
    //                          QString("Saved %1 cropped photos to %2").arg(croppedPhotos.size()).arg(dir));
}

void MainWindow::onFindScannerButtonClicked() {
    if (!scanner) {
        scanner = ScannerInterface::createScanner();
    }

    if (!scanner) {
        QMessageBox::warning(this, "Error", "No suitable scanner backend found!");
        return;
    }

    // std::vector<std::wstring> scanners = scanner->getAvailableScanners();
    std::vector<std::wstring> scanners = scanner->getAvailableScanners();

    if (scanners.empty()) {
        QMessageBox::information(this, "Info", "No scanners found.");
        return;
    }

    // add to combo box
    QStringList scannerList;
    for (const auto &scanner : scanners) {
        scannerList += QString::fromStdWString(scanner);
    }

    ui->comboScanners->addItems(scannerList);
}

void MainWindow::onScannerSelectionChanged(QString scannerName) {
    if (!scanner) {
        QMessageBox::warning(this, "Error", "No suitable scanner backend found!");
        return;
    }

    std::wstring scannerNameW = scannerName.toStdWString();

    std::cout << "Selected scanner: " << scannerName.toStdString() << std::endl;

    scanner->setPreferredScanner(scannerNameW);
}

void MainWindow::displayMatInGraphicsView(const cv::Mat& mat, QGraphicsView* graphicsView) {
    qDebug() << "Mat empty:" << mat.empty();
    qDebug() << "Mat type:" << mat.type();
    qDebug() << "Mat rows:" << mat.rows << ", cols:" << mat.cols;
    // Convert cv::Mat to QImage
    // Handle supported formats
    QImage qImage = matToQImage(mat);

    // Convert QImage to QPixmap
    QPixmap pixmap = QPixmap::fromImage(qImage);

    // Create a scene and add the pixmap
    QGraphicsScene* scene = new QGraphicsScene();
    scene->addPixmap(pixmap);

    // Set the scene to the graphics view
    graphicsView->setScene(scene);

    // Optional: Adjust the view to fit the image
    graphicsView->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
}

QImage MainWindow::matToQImage(const cv::Mat& mat) {
    // Check if the matrix is valid
    if (mat.empty()) {
        qDebug() << "Empty matrix provided to matToQImage.";
        return QImage();
    }

    switch (mat.type()) {
    case CV_8UC1: { // Grayscale
        return QImage(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_Grayscale8).copy();
    }
    case CV_8UC3: { // BGR to RGB
        cv::Mat rgbMat;
        cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);
        return QImage(rgbMat.data, rgbMat.cols, rgbMat.rows, static_cast<int>(rgbMat.step), QImage::Format_RGB888).copy();
    }
    case CV_8UC4: { // BGRA to RGBA
        return QImage(mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_RGBA8888).copy();
    }
    case CV_16UC1: { // 16-bit Grayscale
        // Normalize to 8-bit range [0, 255]
        cv::Mat mat8;
        mat.convertTo(mat8, CV_8UC1, 255.0 / 65535.0); // Scale 16-bit to 8-bit
        return QImage(mat8.data, mat8.cols, mat8.rows, static_cast<int>(mat8.step), QImage::Format_Grayscale8).copy();
    }
    case CV_16UC3: { // 16-bit BGR
        cv::Mat mat8;
        mat.convertTo(mat8, CV_8UC3, 255.0 / 65535.0); // Scale 16-bit to 8-bit
        cv::Mat rgbMat;
        cv::cvtColor(mat8, rgbMat, cv::COLOR_BGR2RGB);
        return QImage(rgbMat.data, rgbMat.cols, rgbMat.rows, static_cast<int>(rgbMat.step), QImage::Format_RGB888).copy();
    }
    default:
        qDebug() << "Unsupported matrix type for QImage conversion: " << mat.type();
        throw std::runtime_error("Unsupported cv::Mat type for QImage conversion.");
    }
}