#pragma once

#include "ScannerInterface.h"

#ifdef _WIN32
#include <windows.h>
// WIA includes are a bit more involved (wia.h, etc.)
// You might need the Windows SDK and other COM-related includes

class WiaScanner : public ScannerInterface
{
public:
    WiaScanner();
    ~WiaScanner() override;

    cv::Mat scanImage() override;

private:
    // Possibly store WIA COM pointers here
};

#endif // _WIN32
