#ifdef _WIN32

#include "WiaScanner.h"

#include <comdef.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <stdexcept>
#include <string>
#include <vector>
#include <wia.h>
#include <wiadef.h>

#define WIA_DPS_HORIZONTAL_RESOLUTION 0x6147
#define WIA_DPS_VERTICAL_RESOLUTION 0x6148

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
        throw std::runtime_error("CoInitialize failed with HRESULT: " + std::to_string(hr));
    }

    std::cout << "initialize: Creating WIA Device Manager instance..." << std::endl;

    hr = CoCreateInstance(CLSID_WiaDevMgr, nullptr, CLSCTX_LOCAL_SERVER, IID_IWiaDevMgr, (void **)&wiaDevMgr);
    if (FAILED(hr)) {
        CoUninitialize();
        throw std::runtime_error("CoCreateInstance failed with HRESULT: " + std::to_string(hr));
    }

    std::cout << "initialize: Populating available scanners..." << std::endl;

    try {
        populateAvailableScanners();
    } catch (const std::exception &e) {
        cleanup();
        CoUninitialize();
        throw std::runtime_error("Exception during populateAvailableScanners: " + std::string(e.what()));
    }

    if (availableScanners.empty()) {
        cleanup();
        CoUninitialize();
        throw std::runtime_error("No scanners found.");
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
    if (!wiaDevMgr)
        throw std::runtime_error("WIA Device Manager not initialized");

    availableScanners.clear(); // Clear existing entries
    IEnumWIA_DEV_INFO *pEnum = nullptr;
    HRESULT hr = wiaDevMgr->EnumDeviceInfo(WIA_DEVINFO_ENUM_LOCAL, &pEnum);

    if (FAILED(hr)) {
        std::cerr << "populateAvailableScanners: EnumDeviceInfo failed with HRESULT: " << std::hex << hr << std::endl;
        throw std::runtime_error("Failed to enumerate WIA devices");
    }

    IWiaPropertyStorage *pStorage = nullptr;
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

void WiaScanner::setPreferredScanner(const std::wstring &scannerName) {
    if (!wiaDevMgr)
        throw std::runtime_error("WIA Device Manager not initialized");

    IWiaItem *device = findDeviceByName(scannerName);
    if (!device)
        throw std::runtime_error("Scanner not found");

    if (selectedDevice)
        selectedDevice->Release();
    selectedDevice = device;

    getDpiConstraints();
    getColorOptions();
}

IWiaItem *WiaScanner::findDeviceByName(const std::wstring &name) {
    std::wcout << L"findDeviceByName: Searching for device with name: " << name << std::endl;

    IEnumWIA_DEV_INFO *pEnum = nullptr;
    HRESULT hr = wiaDevMgr->EnumDeviceInfo(WIA_DEVINFO_ENUM_LOCAL, &pEnum);

    if (FAILED(hr)) {
        std::cerr << "findDeviceByName: EnumDeviceInfo failed with HRESULT: " << std::hex << hr << std::endl;
        return nullptr;
    }

    IWiaPropertyStorage *pStorage = nullptr;
    while (S_OK == pEnum->Next(1, &pStorage, nullptr)) {
        PROPSPEC propSpecName = {PRSPEC_PROPID, WIA_DIP_DEV_NAME};
        PROPVARIANT propVarName = {};
        hr = pStorage->ReadMultiple(1, &propSpecName, &propVarName);

        if (SUCCEEDED(hr) && propVarName.vt == VT_BSTR) {
            std::wcout << L"findDeviceByName: Found device - " << propVarName.bstrVal << std::endl;

            if (_wcsicmp(name.c_str(), propVarName.bstrVal) == 0) {
                std::wcout << L"findDeviceByName: Match found for device - " << propVarName.bstrVal << std::endl;
                IWiaItem *device = nullptr;
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

                if (SUCCEEDED(hr)) {
                    // printDeviceProperties(device); // Call the new function here
                }
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
std::wstring PropVariantToString(const PROPVARIANT &propVar) {
    if (propVar.vt == VT_BSTR) {
        return propVar.bstrVal;
    } else if (propVar.vt == VT_I4) {
        return std::to_wstring(propVar.lVal);
    }
    return L"";
}

// Utility function to create a PROPVARIANT from a std::wstring
PROPVARIANT StringToPropVariant(const std::wstring &value) {
    PROPVARIANT propVar;
    PropVariantInit(&propVar);
    propVar.vt = VT_BSTR;
    propVar.bstrVal = SysAllocString(value.c_str());
    return propVar;
}

// Function to free a PROPVARIANT
void FreePropVariant(PROPVARIANT &propVar) {
    PropVariantClear(&propVar);
}

// Implementation of property getter and setter
std::map<std::wstring, std::wstring> WiaScanner::getAllScannerProperties() {
    if (!selectedDevice)
        throw std::runtime_error("No scanner selected");

    std::map<std::wstring, std::wstring> properties;
    IWiaPropertyStorage *pStorage = nullptr;
    HRESULT hr = selectedDevice->QueryInterface(IID_IWiaPropertyStorage, (void **)&pStorage);
    if (FAILED(hr))
        throw std::runtime_error("Failed to query property storage");

    IEnumSTATPROPSTG *pEnum = nullptr;
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

void WiaScanner::setScannerProperty(const std::wstring &propertyName, const std::wstring &value) {
    if (!selectedDevice)
        throw std::runtime_error("No scanner selected");

    IWiaPropertyStorage *pStorage = nullptr;
    HRESULT hr = selectedDevice->QueryInterface(IID_IWiaPropertyStorage, (void **)&pStorage);
    if (FAILED(hr))
        throw std::runtime_error("Failed to query property storage");

    IEnumSTATPROPSTG *pEnum = nullptr;
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
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) override {
        if (riid == IID_IUnknown || riid == IID_IWiaDataCallback) {
            *ppvObject = static_cast<IWiaDataCallback *>(this);
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
        BYTE *pbBuffer) override {
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

    IEnumWiaItem *enumWiaItem = nullptr;
    HRESULT hr = selectedDevice->EnumChildItems(&enumWiaItem);
    if (FAILED(hr) || !enumWiaItem) {
        throw std::runtime_error("Failed to enumerate child items of the selected device.");
    }

    IWiaItem *scannerItem = nullptr;
    ULONG fetched = 0;
    hr = enumWiaItem->Next(1, &scannerItem, &fetched);
    enumWiaItem->Release(); // Release the enumerator after retrieving the item

    if (FAILED(hr) || !scannerItem) {
        throw std::runtime_error("No scanner item found in the selected device.");
    }

    IWiaDataTransfer *dataTransfer = nullptr;
    hr = scannerItem->QueryInterface(IID_IWiaDataTransfer, (void **)&dataTransfer);
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
    WiaDataCallback *callback = new WiaDataCallback();

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

void WiaScanner::printDeviceProperties(IWiaItem *device) {
    if (!device) {
        std::cerr << "printDeviceProperties: Device is null." << std::endl;
        return;
    }

    IEnumWIA_DEV_CAPS *pEnumCaps = nullptr;
    HRESULT hr = device->EnumDeviceCapabilities(WIA_DEVICE_COMMANDS, &pEnumCaps);

    if (FAILED(hr)) {
        std::cerr << "printDeviceProperties: Failed to enumerate device properties. HRESULT: " << std::hex << hr << std::endl;
        return;
    }

    IWiaPropertyStorage *pStorage = nullptr;
    hr = device->QueryInterface(IID_IWiaPropertyStorage, (void **)&pStorage);
    if (FAILED(hr)) {
        std::cerr << "printDeviceProperties: Failed to get property storage. HRESULT: " << std::hex << hr << std::endl;
        return;
    }

    IEnumSTATPROPSTG *pEnumProps = nullptr;
    hr = pStorage->Enum(&pEnumProps);
    if (FAILED(hr)) {
        std::cerr << "printDeviceProperties: Failed to enumerate properties. HRESULT: " << std::hex << hr << std::endl;
        pStorage->Release();
        return;
    }

    STATPROPSTG propStat = {};
    while (pEnumProps->Next(1, &propStat, nullptr) == S_OK) {
        PROPSPEC propSpec = {PRSPEC_PROPID, propStat.propid};
        PROPVARIANT propVar = {};

        hr = pStorage->ReadMultiple(1, &propSpec, &propVar);
        if (SUCCEEDED(hr)) {
            std::wcout << L"Property ID: " << propStat.propid << L", Name: " << propStat.lpwstrName << L", Value: ";

            switch (propVar.vt) {
            case VT_BSTR:
                std::wcout << propVar.bstrVal;
                break;
            case VT_I4:
                std::wcout << propVar.lVal;
                break;
            case VT_UI4:
                std::wcout << propVar.ulVal;
                break;
            case VT_BOOL:
                std::wcout << (propVar.boolVal ? L"True" : L"False");
                break;
            case VT_EMPTY:
                std::wcout << L"(Empty)";
                break;
            default:
                std::wcout << L"(Unhandled type)";
                break;
            }

            std::wcout << std::endl;
            PropVariantClear(&propVar);
        }

        CoTaskMemFree(propStat.lpwstrName);
    }

    pEnumProps->Release();
    pStorage->Release();
}

void WiaScanner::getDpiConstraints() {
    IWiaItem *device = selectedDevice;

    if (!device) {
        std::cerr << "getResolutionRange: Device is not initialized." << std::endl;
        return;
    }

    IWiaPropertyStorage *pStorage = nullptr;
    HRESULT hr = device->QueryInterface(IID_IWiaPropertyStorage, (void **)&pStorage);
    if (FAILED(hr)) {
        std::cerr << "getResolutionRange: Failed to get property storage. HRESULT: " << std::hex << hr << std::endl;
        return;
    }

    PROPSPEC propSpecs[2] = {
        {PRSPEC_PROPID, WIA_IPS_XRES}, // Horizontal resolution
        {PRSPEC_PROPID, WIA_IPS_YRES}  // Vertical resolution
    };

    for (int i = 0; i < 2; ++i) {
        PROPVARIANT propVar = {};
        ULONG accessFlags = 0;
        PROPVARIANT attrVar = {};

        // Retrieve the property attributes
        hr = pStorage->GetPropertyAttributes(1, &propSpecs[i], &accessFlags, &attrVar);
        if (SUCCEEDED(hr) && attrVar.vt == VT_I4) {
            LONG minValue = LOWORD(attrVar.ulVal); // Minimum resolution
            LONG maxValue = HIWORD(attrVar.ulVal); // Maximum resolution
            LONG stepValue = attrVar.ulVal >> 16;  // Step value (if supported)

            std::wcout << (i == 0 ? L"Horizontal" : L"Vertical") << L" Resolution Range:" << std::endl;
            std::wcout << L"  Min: " << minValue << L" DPI" << std::endl;
            std::wcout << L"  Max: " << maxValue << L" DPI" << std::endl;
            std::wcout << L"  Step: " << stepValue << L" DPI" << std::endl;
        } else {
            std::cerr << "getResolutionRange: Failed to get attributes for "
                      << (i == 0 ? "Horizontal" : "Vertical") << " resolution. HRESULT: " << std::hex << hr << std::endl;
        }

        PropVariantClear(&propVar);
        PropVariantClear(&attrVar);
    }

    if (pStorage) {
        pStorage->Release();
    }
}

void WiaScanner::setDpi(int dpi) {
    IWiaItem *device = selectedDevice;
    if (!device) {
        throw std::runtime_error("Device is not initialized.");
    }

    IWiaPropertyStorage *pStorage = nullptr;
    HRESULT hr = device->QueryInterface(IID_IWiaPropertyStorage, (void **)&pStorage);
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to get property storage. HRESULT: " + std::to_string(hr));
    }

    PROPSPEC propSpec = {PRSPEC_PROPID, WIA_IPS_XRES}; // DPI property
    PROPVARIANT propVar = {};
    propVar.vt = VT_I4;
    propVar.lVal = dpi;

    hr = pStorage->WriteMultiple(1, &propSpec, &propVar, 0);
    if (FAILED(hr)) {
        pStorage->Release();
        throw std::runtime_error("Failed to set DPI. HRESULT: " + std::to_string(hr));
    }

    // Release property storage if successful
    if (pStorage) {
        pStorage->Release();
    }
}


// Function to get color options and the current color setting
void WiaScanner::getColorOptions() {
    IWiaItem *device = selectedDevice;
    if (!device) {
        std::cerr << "getColorOptions: Device is not initialized." << std::endl;
        return;
    }

    IWiaPropertyStorage *pStorage = nullptr;
    HRESULT hr = device->QueryInterface(IID_IWiaPropertyStorage, (void **)&pStorage);
    if (FAILED(hr)) {
        std::cerr << "getColorOptions: Failed to get property storage. HRESULT: " << std::hex << hr << std::endl;
        return;
    }

    PROPSPEC propSpec = {PRSPEC_PROPID, WIA_IPA_DATATYPE}; // Color property
    PROPVARIANT propVar = {};
    hr = pStorage->ReadMultiple(1, &propSpec, &propVar);

    if (SUCCEEDED(hr) && propVar.vt == VT_I4) {
        std::wstring currentSetting;
        switch (propVar.lVal) {
        case WIA_DATA_COLOR:
            currentSetting = L"Color";
            break;
        case WIA_DATA_GRAYSCALE:
            currentSetting = L"Grayscale";
            break;
        case WIA_DATA_THRESHOLD:
            currentSetting = L"Black and White";
            break;
        default:
            currentSetting = L"Unknown";
            break;
        }
        std::wcout << L"Current Color Option: " << currentSetting << std::endl;

        std::wcout << L"Available Color Options: Color, Grayscale, Black and White" << std::endl;
    } else {
        std::cerr << "getColorOptions: Failed to read current color option. HRESULT: " << std::hex << hr << std::endl;
    }

    if (pStorage) {
        PropVariantClear(&propVar);
        pStorage->Release();
    }
}

// Function to set the color option
void WiaScanner::setColorOption(int colorOption) {
    // Map the input to WIA color options
    switch (colorOption) {
    case 1:
        colorOption = WIA_DATA_COLOR;
        break;
    case 2:
        colorOption = WIA_DATA_GRAYSCALE;
        break;
    case 3:
        colorOption = WIA_DATA_THRESHOLD;
        break;
    default:
        throw std::invalid_argument("Invalid color option provided.");
    }

    IWiaItem *device = selectedDevice;
    if (!device) {
        throw std::runtime_error("Device is not initialized.");
    }

    IWiaPropertyStorage *pStorage = nullptr;
    HRESULT hr = device->QueryInterface(IID_IWiaPropertyStorage, (void **)&pStorage);
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to get property storage. HRESULT: " + std::to_string(hr));
    }

    PROPSPEC propSpec = {PRSPEC_PROPID, WIA_IPA_DATATYPE}; // Color property
    PROPVARIANT propVar = {};
    propVar.vt = VT_I4;
    propVar.lVal = colorOption;

    hr = pStorage->WriteMultiple(1, &propSpec, &propVar, 0);
    if (FAILED(hr)) {
        pStorage->Release();
        throw std::runtime_error("Failed to set color option. HRESULT: " + std::to_string(hr));
    }

    // Release storage if successful
    if (pStorage) {
        pStorage->Release();
    }
}

#endif // _WIN32
