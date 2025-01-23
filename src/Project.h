#ifndef PROJECT_H
#define PROJECT_H

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <string>

class Project {
public:
    struct ProjectData {
        std::string projectName;
        std::string projectPath;
        std::string createDate;
        std::string modifiedDate;
        std::string scannerName;
        int scannerDpi;
        int scannerColor;
        int imagesCount;
        int scanOrientation;
        std::string imageDateTime;
        std::pair<double, double> imageLocation;
    };

    // Load project data from JSON file
    static ProjectData loadProject(const std::string &folderPath);

    // Check if a file contains a valid project structure
    static bool checkProject(const std::string &folderPath);

    // Update project data in JSON file
    static bool updateProject(const std::string &folderPath, const ProjectData &data);

    static bool createProject(const std::string &basePath, const std::string &projectName);

private:
    // Helper function to convert ProjectData to QJsonObject
    static QJsonObject toJson(const ProjectData &data);

    // Helper function to convert QJsonObject to ProjectData
    static ProjectData fromJson(const QJsonObject &obj);
};

#endif // PROJECT_H
