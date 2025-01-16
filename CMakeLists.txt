cmake_minimum_required(VERSION 3.15)
project(PichaScan LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt (we’ll assume Qt6; for Qt5, replace Qt6 with Qt5 and adjust versions)
find_package(Qt6 6.5 COMPONENTS Widgets REQUIRED)

# Find OpenCV
set(OpenCV_DIR "C:/opencv-mingw-64/x64/mingw/lib")
find_package(OpenCV REQUIRED)

# (Optional) Find exiv2 for metadata
# find_package(Exiv2 REQUIRED)

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
if (UNIX)
    message(STATUS "Configuring for Linux - will use SANE (placeholder).")
    # Potentially find and link SANE if you want to do it right away:
    # find_library(SANE_LIB sane REQUIRED)
elseif (WIN32)
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

add_executable(${PROJECT_NAME} ${SRC_FILES} ${UI_FILES})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Widgets
        ${OpenCV_LIBS}
        # exiv2 if you include it
        # ${SANE_LIB} if you find it
)

# For Windows, you may need extra linking for WIA or TWAIN
# (TWAIN often just requires linking the TWAIN_32.lib or calling the DLL.)
# For example:
# if(WIN32)
#   target_link_libraries(${PROJECT_NAME} PRIVATE twain_32)
# endif()

if(WIN32)
    # This custom command will run after PichaScan is built
    add_custom_command(TARGET PichaScan
        POST_BUILD
        COMMAND "${Qt6_DIR}/../../../bin/windeployqt.exe"
                --dir "$<TARGET_FILE_DIR:PichaScan>"
                "$<TARGET_FILE:PichaScan>"
        COMMENT "Running windeployqt to deploy Qt libs..."
    )
endif()
