#include "ImageSaver.h"
#include <QDebug>
#include <QFileInfo>
#include <exiv2/exiv2.hpp>

bool ImageSaver::saveImage(const cv::Mat &image, const QString &filePath, const QString &dateTimeString, std::pair<double, double> imageLocation) {
    if (image.empty()) {
        qWarning() << "Image is empty, cannot save to" << filePath;
        return false;
    }

    // Save the image file
    bool res = cv::imwrite(filePath.toStdString(), image);
    if (!res) {
        qWarning() << "Failed to save image to" << filePath;
        return false;
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
        exifData["Exif.Photo.DateTimeOriginal"] = dateTimeString.toStdString();

        // Set GPS Latitude
        double latitude = imageLocation.first;
        exifData["Exif.GPSInfo.GPSLatitude"] = toExifString(latitude, true, true);
        exifData["Exif.GPSInfo.GPSLatitudeRef"] = latitude >= 0 ? "N" : "S";

        // Set GPS Longitude
        double longitude = imageLocation.second;
        exifData["Exif.GPSInfo.GPSLongitude"] = toExifString(longitude, true, false);
        exifData["Exif.GPSInfo.GPSLongitudeRef"] = longitude >= 0 ? "E" : "W";

        // Save the updated metadata back to the file
        exiv_image->writeMetadata();

        std::cout << "Metadata updated successfully." << std::endl;
    } catch (Exiv2::Error &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }

    return res;
}
std::string ImageSaver::toExifString(double d, bool bRational, bool bLat) {
    const char *NS = d >= 0.0 ? "N" : "S";
    const char *EW = d >= 0.0 ? "E" : "W";
    const char *NSEW = bLat ? NS : EW;
    if (d < 0)
        d = -d;
    int deg = static_cast<int>(d);
    d -= deg;
    d *= 60;
    int min = static_cast<int>(d);
    d -= min;
    d *= 60;
    int sec = static_cast<int>(d);
    char result[200];
    if (bRational)
        snprintf(result, sizeof(result), "%d/1 %d/1 %d/1", deg, min, sec);
    else
        snprintf(result, sizeof(result), "%03d°%02d'%02d\"%s", deg, min, sec, NSEW);
    return result;
}