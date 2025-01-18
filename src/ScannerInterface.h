#pragma once

#include <opencv2/opencv.hpp>
#include <memory>

class ScannerInterface
{
public:
    virtual ~ScannerInterface() = default;

    // Getter for available scanners
    virtual std::vector<std::wstring> getAvailableScanners() const = 0;
    // The core function: scan an image from the scanner
    // For multi-page scanning or advanced controls, add more methods
    virtual cv::Mat scanImage() = 0;

    // Get a list of available scanners
    // virtual void populateAvailableScanners();
    virtual void setPreferredScanner(const std::wstring& scannerName) = 0;
    // virtual std::map<std::wstring, std::wstring> getAllScannerProperties();
    // virtual void setScannerProperty(const std::wstring& propertyName, const std::wstring& value);


    // Static factory: returns appropriate scanner for the current platform
    static std::unique_ptr<ScannerInterface> createScanner();
};
