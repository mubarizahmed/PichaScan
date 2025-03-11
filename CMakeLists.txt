cmake_minimum_required(VERSION 3.15)
project(PichaScan LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt (we’ll assume Qt6; for Qt5, replace Qt6 with Qt5 and adjust versions)
set(CMAKE_PREFIX_PATH "C:/Qt/6.8.1/mingw_64/lib")
find_package(Qt6 6.5 COMPONENTS Widgets Qml QuickWidgets Location Positioning REQUIRED)

# Find OpenCV
set(OpenCV_DIR "C:/opencv-mingw-64/x64/mingw/lib")
find_package(OpenCV REQUIRED)

# Find Exiv2
set(EXIV2_DIR "C:/libs/exiv2-0.27.5-MinGW64/lib/cmake/exiv2")
find_package(EXIV2 REQUIRED)
if(EXIV2_FOUND)
    message(STATUS "Exiv2 found: ${EXIV2_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "Exiv2 not found")
endif()

include_directories("C:/libs/exiv2-0.27.5-MinGW64/include")

# Get the API key from an environment variable or define it in CMake
if(DEFINED ENV{MAP_API_KEY})
    set(API_KEY $ENV{MAP_API_KEY})
endif()

# Process the QML template to replace @API_KEY@
configure_file(
    ${CMAKE_SOURCE_DIR}/src/qml/map.qml # Input template
    ${CMAKE_BINARY_DIR}/map.qml # Output QML file with replaced key
    @ONLY # Only replace @...@ placeholders
)

# If you have a .ui file, be sure to enable Qt’s AUTOUIC
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Configure output directories for different build types
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/release")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/debug")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/release")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/debug")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/release")

# You can also detect the platform to decide if you want SANE or WIA/TWAIN, etc.
if(UNIX)
    message(STATUS "Configuring for Linux - will use SANE (placeholder).")
    # Potentially find and link SANE if you want to do it right away:
    # find_library(SANE_LIB sane REQUIRED)
elseif(WIN32)
    message(STATUS "Configuring for Windows - will use WIA/TWAIN (placeholder).")
    # Link or find WIA/TWAIN libs as needed
endif()

# Add the executable
file(GLOB SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

file(GLOB UI_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/ui/*.ui"
)

if(WIN32)
    set(APP_ICON_RESOURCE "${CMAKE_BINARY_DIR}/Logomark.rc")
    configure_file("${CMAKE_SOURCE_DIR}/res/Logomark.ico" "${CMAKE_BINARY_DIR}/Logomark.ico" COPYONLY)
    configure_file("${CMAKE_SOURCE_DIR}/res/Logomark.rc" "${APP_ICON_RESOURCE}" COPYONLY)
    set(SRC_FILES ${SRC_FILES} ${APP_ICON_RESOURCE})
endif()

if(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${SRC_FILES} ${UI_FILES} "${CMAKE_BINARY_DIR}/Logomark.rc")
else()
    add_executable(${PROJECT_NAME} ${SRC_FILES} ${UI_FILES})
endif()

set(qml_resource_files
    "${CMAKE_BINARY_DIR}/map.qml"
    # "${CMAKE_CURRENT_SOURCE_DIR}/res.qrc"
    # "${CMAKE_CURRENT_SOURCE_DIR}/res/Logomark.svg"
)

qt6_add_resources(${PROJECT_NAME} "resources"
    FILES "res.qrc"
    "res/Logomark.svg"
)

set_source_files_properties(${qml_resource_files} PROPERTIES
    QT_RESOURCE_ALIAS "map.qml"
)

# set(qml_resource_files
#     "${CMAKE_BINARY_DIR}/map.qml"
# )

qt6_add_resources(${PROJECT_NAME} "qml"
    PREFIX
    "/"
    FILES
    ${qml_resource_files}
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt6::Location
    Qt6::Positioning
    Qt6::Widgets
    Qt6::QuickWidgets
    Qt6::Qml
    Qt6::Gui

    ${OpenCV_LIBS}
    # ${EXIV2_LIBS}
    # ${SANE_LIB} if you find it
    C:/libs/exiv2-0.27.5-MinGW64/lib/libexiv2.dll.a
    ${EXIV2_LIBS}
)



if(WIN32)
    # Paths to the required DLLs
    set(EXIV2_DLL "C:/libs/exiv2-0.27.5-MinGW64/bin/libexiv2.dll")
    set(ZLIB_DLL "C:/msys64/ucrt64/bin/zlib1.dll")
    set(EXPAT_DLL "C:/msys64/ucrt64/bin/libexpat-1.dll")
    set(ICONV_DLL "C:/msys64/ucrt64/bin/libiconv-2.dll")
    set(INTL_DLL "C:/msys64/ucrt64/bin/libintl-8.dll")

    # Custom command to deploy Qt libraries and other dependencies
    add_custom_command(TARGET PichaScan
        POST_BUILD
        COMMAND "${Qt6_DIR}/../../../bin/windeployqt.exe"
        --dir "$<TARGET_FILE_DIR:PichaScan>"
        --qmldir "${CMAKE_BINARY_DIR}"
        "$<TARGET_FILE:PichaScan>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${EXIV2_DLL}"
        "$<TARGET_FILE_DIR:PichaScan>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${ZLIB_DLL}"
        "$<TARGET_FILE_DIR:PichaScan>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${EXPAT_DLL}"
        "$<TARGET_FILE_DIR:PichaScan>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${ICONV_DLL}"
        "$<TARGET_FILE_DIR:PichaScan>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${INTL_DLL}"
        "$<TARGET_FILE_DIR:PichaScan>"
        COMMENT "Deploying Qt libs and copying Exiv2 and other dependency DLLs..."
    )
endif()
