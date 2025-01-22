#include "ImageSaver.h"
#include <QDebug>
#include <QFileInfo>
#include <exiv2/exiv2.hpp>

bool ImageSaver::saveImage(const cv::Mat &image, const QString &filePath, const QString &dateTimeString) {
    if (image.empty()) {
        qWarning() << "Image is empty, cannot save to" << filePath;
        return false;
    }
    // We can use OpenCV's imwrite, or convert to QImage and use QImage::save
    bool res = cv::imwrite(filePath.toStdString(), image);

    if (!res) {
        qWarning() << "Failed to save image to" << filePath;
    } else {
        qDebug() << "Image saved to" << filePath;
    }

    try {

        // Load the image file using Exiv2
        Exiv2::Image::AutoPtr exiv_image = Exiv2::ImageFactory::open(filePath.toStdString());
        if (!exiv_image.get()) {
            throw Exiv2::Error(Exiv2::kerErrorMessage, "Failed to open image file.");
        }

        // Read the existing metadata
        exiv_image->readMetadata();

        // Access the Exif data
        Exiv2::ExifData &exifData = exiv_image->exifData();

        // Write the DateTimeOriginal tag
        std::string tagKey = "Exif.Photo.DateTimeOriginal";
        exifData[tagKey] = dateTimeString.toStdString();

        // Save the updated metadata back to the file
        exiv_image->writeMetadata();

        std::cout << "Metadata updated successfully." << std::endl;
    } catch (Exiv2::Error &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return res;
}