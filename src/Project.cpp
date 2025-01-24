#include "Project.h"
#include <QDir>
#include <QFileInfo>

bool Project::createProject(const std::string &basePath, const std::string &projectName) {
    // Construct folder path
    QString folderPath = QString::fromStdString(basePath);


    // Initialize default project data
    ProjectData data;
    data.projectName = projectName;
    data.projectPath = folderPath.toStdString();
    data.createDate = QDateTime::currentDateTime().toString(Qt::ISODate).toStdString();
    data.modifiedDate = data.createDate; // Same as createDate initially
    data.scannerName = "Default Scanner";
    data.scannerDpi = 300;
    data.scannerColor = 1;
    data.scanOrientation = 0;
    data.imagesCount = 0;
    data.imageDateTime = QDateTime::currentDateTime().toString("yyyy:MM:dd HH:mm:ss").toStdString();
    data.imageLocation = {-0.2894057652214143, 36.081968557610146};

    // Write the JSON file
    return updateProject(basePath, data);
}

// Convert ProjectData to QJsonObject
QJsonObject Project::toJson(const ProjectData &data) {
    QJsonObject obj;
    obj["projectName"] = QString::fromStdString(data.projectName);
    obj["projectPath"] = QString::fromStdString(data.projectPath);
    obj["createDate"] = QString::fromStdString(data.createDate);
    obj["modifiedDate"] = QString::fromStdString(data.modifiedDate);
    obj["scannerName"] = QString::fromStdString(data.scannerName);
    obj["scannerDpi"] = data.scannerDpi;
    obj["scannerColor"] = data.scannerColor;
    obj["imagesCount"] = data.imagesCount;
    obj["scanOrientation"] = data.scanOrientation;
    obj["imageDateTime"] = QString::fromStdString(data.imageDateTime);
    // Create a JSON object for imageLocation
    QJsonObject imageLocationObj;
    imageLocationObj["lat"] = data.imageLocation.first;
    imageLocationObj["lon"] = data.imageLocation.second;
    obj["imageLocation"] = imageLocationObj;

    return obj;
}

// Convert QJsonObject to ProjectData
Project::ProjectData Project::fromJson(const QJsonObject &obj) {
    ProjectData data;
    data.projectName = obj["projectName"].toString().toStdString();
    data.projectPath = obj["projectPath"].toString().toStdString();
    data.createDate = obj["createDate"].toString().toStdString();
    data.modifiedDate = obj["modifiedDate"].toString().toStdString();
    data.scannerName = obj["scannerName"].toString().toStdString();
    data.scannerDpi = obj["scannerDpi"].toInt();
    data.scannerColor = obj["scannerColor"].toInt();
    data.imagesCount = obj["imagesCount"].toInt();
    data.scanOrientation = obj["scanOrientation"].toInt();
    data.imageDateTime = obj["imageDateTime"].toString().toStdString();

    // Parse imageLocation as a JSON object
    if (obj.contains("imageLocation") && obj["imageLocation"].isObject()) {
        QJsonObject imageLocationObj = obj["imageLocation"].toObject();
        data.imageLocation.first = imageLocationObj["lat"].toDouble();
        data.imageLocation.second = imageLocationObj["lon"].toDouble();
    } else {
        // Handle missing or invalid imageLocation
        data.imageLocation = std::make_pair(0.0, 0.0); // Default values
    }

    return data;
}

// Load project data from JSON file
Project::ProjectData Project::loadProject(const std::string &folderPath) {
    std::string filePath = folderPath + "/project.json";
    qDebug() << "loadProject: " << filePath;
    QFile file(QString::fromStdString(filePath));
    if (!file.open(QIODevice::ReadOnly)) {
        throw std::runtime_error("Unable to open file: " + filePath);
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        throw std::runtime_error("Invalid JSON structure in file: " + filePath);
    }

    return fromJson(doc.object());
}

// Check if a file contains a valid project structure
bool Project::checkProject(const std::string &folderPath) {
    std::string filePath = folderPath + "/project.json";
    QFile file(QString::fromStdString(filePath));
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return false;
    }

    QJsonObject obj = doc.object();

    // Check required fields
    if (!(obj.contains("projectName") && obj.contains("projectPath") &&
          obj.contains("createDate") && obj.contains("modifiedDate") &&
          obj.contains("scannerName") && obj.contains("scannerDpi") &&
          obj.contains("scannerColor") && obj.contains("imagesCount") &&
          obj.contains("scanOrientation") && obj.contains("imageLocation"))) {
        return false;
    }

    // Validate imageLocation structure
    if (!obj["imageLocation"].isObject()) {
        return false;
    }

    QJsonObject imageLocationObj = obj["imageLocation"].toObject();
    if (!imageLocationObj.contains("lat") || !imageLocationObj.contains("lon") ||
        !imageLocationObj["lat"].isDouble() || !imageLocationObj["lon"].isDouble()) {
        return false;
    }

    return true;
}

// Update project data in JSON file
bool Project::updateProject(const std::string &folderPath, const ProjectData &data) {
    std::string filePath = folderPath + "/project.json";
    QFile file(QString::fromStdString(filePath));
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonObject obj = toJson(data);
    QJsonDocument doc(obj);
    file.write(doc.toJson());
    file.close();

    return true;
}
