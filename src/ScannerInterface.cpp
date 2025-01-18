#include "ScannerInterface.h"
#ifdef __linux__
#include "SaneScanner.h"
#endif

#ifdef _WIN32
#include "WiaScanner.h"
// #include "TwainScanner.h"
#endif

std::unique_ptr<ScannerInterface> ScannerInterface::createScanner()
{
#ifdef __linux__
    return std::make_unique<SaneScanner>();
#elif defined(_WIN32)
    // WiaScanner* wiaScanner = new WiaScanner();
    // std::vector<std::wstring> scanners2 = wiaScanner->availableScanners;
    // std::vector<std::wstring> scanners = wiaScanner->getAvailableScanners();
    // if (wiaScanner->getAvailableScanners().empty()) {
    //     std::cerr << "No scanners found during creation." << std::endl;
    //     return nullptr;
    // }
    return std::make_unique<WiaScanner>();

    // return std::make_unique<WiaScanner>();
    // Or if you prefer TWAIN, return std::make_unique<TwainScanner>();
#else
    return nullptr;
#endif
}
