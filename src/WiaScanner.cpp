#ifdef _WIN32

#include "WiaScanner.h"

#include <wia.h>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <opencv2/opencv.hpp>

#define MIN_PROPID 2



WiaScanner::WiaScanner() {
    if (FAILED(initialize())) {
        throw std::runtime_error("Failed to initialize WIA Scanner");
    }
}

WiaScanner::~WiaScanner() {
    cleanup();
}

HRESULT WiaScanner::initialize() {
    std::cout << "initialize: Starting COM initialization..." << std::endl;
    HRESULT hr = CoInitialize(nullptr);
    if (FAILED(hr)) {
        std::cerr << "initialize: CoInitialize failed with HRESULT: " << std::hex << hr << std::endl;
        return hr;
    }

    std::cout << "initialize: Creating WIA Device Manager instance..." << std::endl;
    hr = CoCreateInstance(CLSID_WiaDevMgr, nullptr, CLSCTX_LOCAL_SERVER, IID_IWiaDevMgr, (void**)&wiaDevMgr);

    if (FAILED(hr)) {
        std::cerr << "initialize: CoCreateInstance failed with HRESULT: " << std::hex << hr << std::endl;
        CoUninitialize();
        return hr;
    }

    std::cout << "initialize: Populating available scanners..." << std::endl;
    try {
        populateAvailableScanners();
    } catch (const std::exception& e) {
        std::cerr << "initialize: Exception during populateAvailableScanners - " << e.what() << std::endl;
        cleanup();
        return E_FAIL;
    }

    if (availableScanners.empty()) {
        std::cerr << "initialize: No scanners found!" << std::endl;
        cleanup();
        return E_FAIL;
    }

    std::cout << "initialize: Selecting the first scanner..." << std::endl;
    selectedDevice = findDeviceByName(availableScanners.at(0));
    if (!selectedDevice) {
        std::cerr << "initialize: Failed to find device by name." << std::endl;
        cleanup();
        return E_FAIL;
    }

    std::cout << "initialize: Initialization complete." << std::endl;
    return hr;
}

void WiaScanner::cleanup() {
    if (selectedDevice) {
        std::cout << "cleanup: Releasing selected device..." << std::endl;
        selectedDevice->Release();
        selectedDevice = nullptr;
    }
    if (wiaDevMgr) {
        std::cout << "cleanup: Releasing WIA Device Manager..." << std::endl;
        wiaDevMgr->Release();
        wiaDevMgr = nullptr;
    }
    std::cout << "cleanup: Uninitializing COM..." << std::endl;
    CoUninitialize();
    std::cout << "cleanup: Completed cleanup." << std::endl;
}


void WiaScanner::populateAvailableScanners() {
    if (!wiaDevMgr) throw std::runtime_error("WIA Device Manager not initialized");

    availableScanners.clear(); // Clear existing entries
    IEnumWIA_DEV_INFO* pEnum = nullptr;
    HRESULT hr = wiaDevMgr->EnumDeviceInfo(WIA_DEVINFO_ENUM_LOCAL, &pEnum);

    if (FAILED(hr)) {
        std::cerr << "populateAvailableScanners: EnumDeviceInfo failed with HRESULT: " << std::hex << hr << std::endl;
        throw std::runtime_error("Failed to enumerate WIA devices");
    }

    IWiaPropertyStorage* pStorage = nullptr;
    while (S_OK == pEnum->Next(1, &pStorage, nullptr)) {
        PROPSPEC propSpec[1] = {};
        PROPVARIANT propVar[1] = {};

        propSpec[0].ulKind = PRSPEC_PROPID;
        propSpec[0].propid = WIA_DIP_DEV_NAME;

        hr = pStorage->ReadMultiple(1, propSpec, propVar);
        if (SUCCEEDED(hr) && propVar[0].vt == VT_BSTR) {
            availableScanners.push_back(propVar[0].bstrVal);
            std::wcout << L"Found scanner: " << propVar[0].bstrVal << std::endl;
            PropVariantClear(&propVar[0]);
        }
        pStorage->Release();
    }
    pEnum->Release();

    if (availableScanners.empty()) {
        std::cerr << "populateAvailableScanners: No scanners found." << std::endl;
    } else {
        std::cout << "populateAvailableScanners: Total scanners found: " << availableScanners.size() << std::endl;
    }
}

std::vector<std::wstring> WiaScanner::getAvailableScanners() const {
    return availableScanners;
};

void WiaScanner::setPreferredScanner(const std::wstring& scannerName) {
    if (!wiaDevMgr) throw std::runtime_error("WIA Device Manager not initialized");

    IWiaItem* device = findDeviceByName(scannerName);
    if (!device) throw std::runtime_error("Scanner not found");

    if (selectedDevice) selectedDevice->Release();
    selectedDevice = device;
}

IWiaItem* WiaScanner::findDeviceByName(const std::wstring& name) {
    std::wcout << L"findDeviceByName: Searching for device with name: " << name << std::endl;

    IEnumWIA_DEV_INFO* pEnum = nullptr;
    HRESULT hr = wiaDevMgr->EnumDeviceInfo(WIA_DEVINFO_ENUM_LOCAL, &pEnum);

    if (FAILED(hr)) {
        std::cerr << "findDeviceByName: EnumDeviceInfo failed with HRESULT: " << std::hex << hr << std::endl;
        return nullptr;
    }

    IWiaPropertyStorage* pStorage = nullptr;
    while (S_OK == pEnum->Next(1, &pStorage, nullptr)) {
        PROPSPEC propSpecName = {PRSPEC_PROPID, WIA_DIP_DEV_NAME};
        PROPVARIANT propVarName = {};
        hr = pStorage->ReadMultiple(1, &propSpecName, &propVarName);

        if (SUCCEEDED(hr) && propVarName.vt == VT_BSTR) {
            std::wcout << L"findDeviceByName: Found device - " << propVarName.bstrVal << std::endl;

            if (_wcsicmp(name.c_str(), propVarName.bstrVal) == 0) {
                std::wcout << L"findDeviceByName: Match found for device - " << propVarName.bstrVal << std::endl;
                IWiaItem* device = nullptr;
                PROPSPEC propSpecId = {PRSPEC_PROPID, WIA_DIP_DEV_ID};
                PROPVARIANT propVarId = {};

                hr = pStorage->ReadMultiple(1, &propSpecId, &propVarId);
                if (SUCCEEDED(hr) && propVarId.vt == VT_BSTR) {
                    hr = wiaDevMgr->CreateDevice(propVarId.bstrVal, &device);
                    PropVariantClear(&propVarId);
                }
                PropVariantClear(&propVarName);
                pStorage->Release();
                pEnum->Release();
                return SUCCEEDED(hr) ? device : nullptr;
            }
            PropVariantClear(&propVarName);
        }
        pStorage->Release();
    }
    pEnum->Release();

    std::cerr << "findDeviceByName: No match found for device name." << std::endl;
    return nullptr;
}


// Utility function to convert a PROPVARIANT to std::wstring
std::wstring PropVariantToString(const PROPVARIANT& propVar) {
    if (propVar.vt == VT_BSTR) {
        return propVar.bstrVal;
    } else if (propVar.vt == VT_I4) {
        return std::to_wstring(propVar.lVal);
    }
    return L"";
}

// Utility function to create a PROPVARIANT from a std::wstring
PROPVARIANT StringToPropVariant(const std::wstring& value) {
    PROPVARIANT propVar;
    PropVariantInit(&propVar);
    propVar.vt = VT_BSTR;
    propVar.bstrVal = SysAllocString(value.c_str());
    return propVar;
}

// Function to free a PROPVARIANT
void FreePropVariant(PROPVARIANT& propVar) {
    PropVariantClear(&propVar);
}

// Implementation of property getter and setter
std::map<std::wstring, std::wstring> WiaScanner::getAllScannerProperties() {
    if (!selectedDevice) throw std::runtime_error("No scanner selected");

    std::map<std::wstring, std::wstring> properties;
    IWiaPropertyStorage* pStorage = nullptr;
    HRESULT hr = selectedDevice->QueryInterface(IID_IWiaPropertyStorage, (void**)&pStorage);
    if (FAILED(hr)) throw std::runtime_error("Failed to query property storage");

    IEnumSTATPROPSTG* pEnum = nullptr;
    hr = pStorage->Enum(&pEnum);
    if (FAILED(hr)) {
        pStorage->Release();
        throw std::runtime_error("Failed to enumerate properties");
    }

    STATPROPSTG statProp = {};
    while (S_OK == pEnum->Next(1, &statProp, nullptr)) {
        PROPVARIANT propVar;
        PropVariantInit(&propVar);

        PROPSPEC propSpec = {};
        propSpec.ulKind = PRSPEC_PROPID;
        propSpec.propid = statProp.propid;

        HRESULT hrRead = pStorage->ReadMultiple(1, &propSpec, &propVar);
        if (SUCCEEDED(hrRead)) {
            properties[statProp.lpwstrName] = PropVariantToString(propVar);
        }

        FreePropVariant(propVar);
        CoTaskMemFree(statProp.lpwstrName);
    }

    pEnum->Release();
    pStorage->Release();

    return properties;
}

void WiaScanner::setScannerProperty(const std::wstring& propertyName, const std::wstring& value) {
    if (!selectedDevice) throw std::runtime_error("No scanner selected");

    IWiaPropertyStorage* pStorage = nullptr;
    HRESULT hr = selectedDevice->QueryInterface(IID_IWiaPropertyStorage, (void**)&pStorage);
    if (FAILED(hr)) throw std::runtime_error("Failed to query property storage");

    IEnumSTATPROPSTG* pEnum = nullptr;
    hr = pStorage->Enum(&pEnum);
    if (FAILED(hr)) {
        pStorage->Release();
        throw std::runtime_error("Failed to enumerate properties");
    }

    STATPROPSTG statProp = {};
    bool propertyFound = false;
    while (S_OK == pEnum->Next(1, &statProp, nullptr)) {
        if (propertyName == statProp.lpwstrName) {
            PROPSPEC propSpec = {};
            propSpec.ulKind = PRSPEC_PROPID;
            propSpec.propid = statProp.propid;

            PROPVARIANT propVar = StringToPropVariant(value);

            HRESULT hrWrite = pStorage->WriteMultiple(1, &propSpec, &propVar, MIN_PROPID);
            FreePropVariant(propVar);

            if (SUCCEEDED(hrWrite)) {
                propertyFound = true;
            } else {
                CoTaskMemFree(statProp.lpwstrName);
                pEnum->Release();
                pStorage->Release();
                throw std::runtime_error("Failed to set property");
            }
            break;
        }
        CoTaskMemFree(statProp.lpwstrName);
    }

    pEnum->Release();
    pStorage->Release();

    if (!propertyFound) {
        throw std::runtime_error("Property not found");
    }
}

// Minimal IWiaDataCallback implementation
class WiaDataCallback : public IWiaDataCallback {
public:
    ULONG STDMETHODCALLTYPE AddRef() override { return ++m_refCount; }
    ULONG STDMETHODCALLTYPE Release() override {
        if (--m_refCount == 0) {
            delete this;
            return 0;
        }
        return m_refCount;
    }
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override {
        if (riid == IID_IUnknown || riid == IID_IWiaDataCallback) {
            *ppvObject = static_cast<IWiaDataCallback*>(this);
            AddRef();
            return S_OK;
        }
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }
    HRESULT STDMETHODCALLTYPE BandedDataCallback(
        LONG lMessage,
        LONG lStatus,
        LONG lPercentComplete,
        LONG lOffset,
        LONG lLength,
        LONG lReserved,
        LONG lResLength,
        BYTE* pbBuffer
    ) override {
        // For now, just ignore the callback. Implement if needed.
        return S_OK;
    }

private:
    ULONG m_refCount = 1;
};

// scanImage implementation
cv::Mat WiaScanner::scanImage() {
    if (!selectedDevice) {
        throw std::runtime_error("No scanner selected");
    }

    // IWiaItem* scannerItem = nullptr;
    // HRESULT hr = selectedDevice->EnumChildItems(&scannerItem);

    // if (FAILED(hr) || !scannerItem) {
    //     throw std::runtime_error("Failed to access scanner item");
    // }

    IEnumWiaItem* enumWiaItem = nullptr;
    HRESULT hr = selectedDevice->EnumChildItems(&enumWiaItem);
    if (FAILED(hr) || !enumWiaItem) {
        throw std::runtime_error("Failed to enumerate child items of the selected device.");
    }

    IWiaItem* scannerItem = nullptr;
    ULONG fetched = 0;
    hr = enumWiaItem->Next(1, &scannerItem, &fetched);
    enumWiaItem->Release(); // Release the enumerator after retrieving the item

    if (FAILED(hr) || !scannerItem) {
        throw std::runtime_error("No scanner item found in the selected device.");
    }

    IWiaDataTransfer* dataTransfer = nullptr;
    hr = scannerItem->QueryInterface(IID_IWiaDataTransfer, (void**)&dataTransfer);
    scannerItem->Release();

    if (FAILED(hr)) {
        throw std::runtime_error("Failed to get IWiaDataTransfer interface");
    }

    // Prepare STGMEDIUM for file-based transfer
    STGMEDIUM stgMedium = {};
    stgMedium.tymed = TYMED_FILE; // Transfer to a temporary file
    wchar_t tempFilePath[MAX_PATH];
    GetTempPath(MAX_PATH, tempFilePath);
    wcscat_s(tempFilePath, L"wia_scan.bmp");
    stgMedium.lpszFileName = tempFilePath;

    // Create a minimal data callback
    WiaDataCallback* callback = new WiaDataCallback();

    // Perform data transfer
    hr = dataTransfer->idtGetData(&stgMedium, callback);
    dataTransfer->Release();
    callback->Release();

    if (FAILED(hr)) {
        throw std::runtime_error("Failed to transfer image data");
    }

    std::wstring widePath = tempFilePath;
    std::string narrowPath(widePath.begin(), widePath.end());


    // Load the scanned file into OpenCV
    cv::Mat image = cv::imread(narrowPath, cv::IMREAD_COLOR);
    if (image.empty()) {
        throw std::runtime_error("Failed to load scanned image");
    }

    // Clean up temporary file
    DeleteFile(tempFilePath);

    return image;
}

// cv::Mat WiaScanner::scanImage() {
//     if (!selectedDevice) {
//         throw std::runtime_error("No scanner selected");
//     }

//     // Step 1: Enumerate child items
//     IEnumWiaItem* enumWiaItem = nullptr;
//     HRESULT hr = selectedDevice->EnumChildItems(&enumWiaItem);
//     if (FAILED(hr) || !enumWiaItem) {
//         throw std::runtime_error("Failed to enumerate child items of the selected device.");
//     }

//     // Step 2: Retrieve the first child item (assuming it's the scanner item)
//     IWiaItem* scannerItem = nullptr;
//     ULONG fetched = 0;
//     hr = enumWiaItem->Next(1, &scannerItem, &fetched);
//     enumWiaItem->Release(); // Release the enumerator after retrieving the item

//     if (FAILED(hr) || !scannerItem) {
//         throw std::runtime_error("No scanner item found in the selected device.");
//     }

//     // Step 3: Use the scanner item to transfer image data
//     IWiaDataTransfer* dataTransfer = nullptr;
//     hr = scannerItem->QueryInterface(IID_IWiaDataTransfer, (void**)&dataTransfer);
//     scannerItem->Release(); // Release scannerItem after obtaining data transfer interface

//     if (FAILED(hr)) {
//         throw std::runtime_error("Failed to get IWiaDataTransfer interface from the scanner item.");
//     }

//     // Step 4: Set up a memory stream for data transfer
//     IStream* memoryStream = nullptr;
//     hr = CreateStreamOnHGlobal(nullptr, TRUE, &memoryStream);
//     if (FAILED(hr)) {
//         dataTransfer->Release();
//         throw std::runtime_error("Failed to create memory stream for image transfer.");
//     }

//     WIA_DATA_TRANSFER_INFO transferInfo = {};
//     transferInfo.ulSize = sizeof(WIA_DATA_TRANSFER_INFO);
//     transferInfo.ulBufferSize = 1024 * 1024; // Adjust buffer size if needed

//     hr = dataTransfer->idtGetData(&transferInfo, memoryStream);
//     dataTransfer->Release();

//     if (FAILED(hr)) {
//         memoryStream->Release();
//         throw std::runtime_error("Image transfer failed.");
//     }

//     // Step 5: Convert memory stream to cv::Mat
//     LARGE_INTEGER li = {0};
//     ULARGE_INTEGER uli = {0};
//     hr = memoryStream->Seek(li, STREAM_SEEK_END, &uli);
//     if (FAILED(hr)) {
//         memoryStream->Release();
//         throw std::runtime_error("Failed to seek to end of memory stream.");
//     }

//     size_t imageSize = static_cast<size_t>(uli.QuadPart);
//     hr = memoryStream->Seek(li, STREAM_SEEK_SET, nullptr);
//     if (FAILED(hr)) {
//         memoryStream->Release();
//         throw std::runtime_error("Failed to reset memory stream position.");
//     }

//     std::vector<BYTE> buffer(imageSize);
//     ULONG bytesRead = 0;
//     hr = memoryStream->Read(buffer.data(), imageSize, &bytesRead);
//     memoryStream->Release();

//     if (FAILED(hr) || bytesRead != imageSize) {
//         throw std::runtime_error("Failed to read image data from memory stream.");
//     }

//     cv::Mat image = cv::imdecode(buffer, cv::IMREAD_COLOR);
//     if (image.empty()) {
//         throw std::runtime_error("Failed to decode image data.");
//     }

//     return image;
// }

#endif // _WIN32
