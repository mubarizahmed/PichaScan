#pragma once

#include <opencv2/opencv.hpp>
#include <memory>

class ScannerInterface
{
public:
    virtual ~ScannerInterface() = default;

    // The core function: scan an image from the scanner
    // For multi-page scanning or advanced controls, add more methods
    virtual cv::Mat scanImage() = 0;

    // Static factory: returns appropriate scanner for the current platform
    static std::unique_ptr<ScannerInterface> createScanner();
};
