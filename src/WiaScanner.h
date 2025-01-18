#pragma once

#include "ScannerInterface.h"

#ifndef WIA_SCANNER_H
#define WIA_SCANNER_H

#ifdef _WIN32

#include <windows.h>
#include <wia.h>
#include <string>
#include <vector>
#include <map>
#include <opencv2/opencv.hpp>

class WiaScanner : public ScannerInterface{
public:
    WiaScanner();                              // Constructor
    ~WiaScanner() override;                             // Destructor
    std::vector<std::wstring> availableScanners;
    

    std::vector<std::wstring> getAvailableScanners() const override;
    void populateAvailableScanners();
    void setPreferredScanner(const std::wstring& scannerName) override;
    std::map<std::wstring, std::wstring> getAllScannerProperties() ;
    void setScannerProperty(const std::wstring& propertyName, const std::wstring& value) ;
    cv::Mat scanImage() override;

private:
    IWiaDevMgr* wiaDevMgr = nullptr;
    IWiaItem* selectedDevice = nullptr;
    

    IWiaItem* findDeviceByName(const std::wstring& name);
    HRESULT initialize();
    void cleanup();
};

#endif // _WIN32

#endif // WIA_SCANNER_H

