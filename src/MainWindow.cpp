#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "ScannerInterface.h"
#include "ScanProcessor.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Suppose you have a QPushButton named scanButton in your .ui
    connect(ui->scanButton, &QPushButton::clicked,
            this, &MainWindow::onScanButtonClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onScanButtonClicked()
{
    // Get a scanner instance (platform-specific under the hood)
    std::unique_ptr<ScannerInterface> scanner = ScannerInterface::createScanner();

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

    // Process the scanned image to find multiple photos
    ScanProcessor processor;
    std::vector<cv::Mat> croppedPhotos = processor.detectAndCropPhotos(scannedImage);

    if (croppedPhotos.empty()) {
        QMessageBox::information(this, "Info", "No distinct photos detected.");
        return;
    }

    // For demonstration, let’s just save them out with a file dialog
    // In reality, you might do more advanced UI or metadata tagging
    QString dir = QFileDialog::getExistingDirectory(this, "Choose Save Directory");
    if (dir.isEmpty()) {
        return;
    }

    for (size_t i = 0; i < croppedPhotos.size(); i++) {
        QString filename = QString("%1/photo_%2.jpg").arg(dir).arg(i);
        // Convert from cv::Mat to a QImage if you want to use Qt’s saving, or use OpenCV’s imwrite directly
        cv::imwrite(filename.toStdString(), croppedPhotos[i]);
    }

    QMessageBox::information(this, "Success",
        QString("Saved %1 cropped photos to %2").arg(croppedPhotos.size()).arg(dir));
}
