#include "StartWindow.h"
#include "ui_StartWindow.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QDir>

#include "Project.h"

StartWindow::StartWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::StartWindow) {

    ui->setupUi(this);

    connect(ui->btnNewProj, &QPushButton::clicked, this, &StartWindow::onNewProjClicked);
    connect(ui->btnOpenProj, &QPushButton::clicked, this, &StartWindow::onOpenProjClicked);

    show();
}

StartWindow::~StartWindow() {
    delete ui;
}

void StartWindow::onNewProjClicked() {
    // Open a dialog to select a folder
    QString selectedFolder = QFileDialog::getExistingDirectory(
        this, 
        "Select Project Folder", 
        QDir::homePath() + "/Pictures/PichaScan/",
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );

    // Check if the user canceled the folder selection
    if (selectedFolder.isEmpty()) {
        return; // User canceled
    }

    // Prompt the user for a project name
    bool ok;
    QString projectName = QInputDialog::getText(
        this, 
        "Enter Project Name", 
        "Project Name:", 
        QLineEdit::Normal, 
        "", 
        &ok
    );

    // Check if the user canceled or left the input empty
    if (!ok || projectName.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Project name cannot be empty.");
        return;
    }

    // Check if a folder with the project name already exists
    QDir projectDir(selectedFolder);
    if (projectDir.exists(projectName)) {
        QMessageBox::critical(this, "Error", "A project with this name already exists in the selected folder.");
        return;
    }

    // Create the new project folder
    if (projectDir.mkdir(projectName)) {
        QMessageBox::information(this, "Success", "Project created successfully!");

        QString projectPath = projectDir.absoluteFilePath(projectName);
        // Convert to std::string if needed
        std::string projectPathStd = projectPath.toStdString();

        Project::createProject(projectPathStd, projectName.toStdString());

        emit projectOpen(projectPathStd);
    } else {
        QMessageBox::critical(this, "Error", "Failed to create the project folder. Please try again.");
    }
}

void StartWindow::onOpenProjClicked() {
    // Open a dialog to select a project.json file
    QString selectedFile = QFileDialog::getOpenFileName(
        this,
        "Select Project File",
        QDir::homePath(),
        "Project Files (project.json)"
    );

    // Check if the user canceled the file selection
    if (selectedFile.isEmpty()) {
        return; // User canceled
    }

    // Get the folder path from the selected file
    QFileInfo fileInfo(selectedFile);
    QString folderPath = fileInfo.absolutePath();

    // Convert folder path to std::string
    std::string folderPathStd = folderPath.toStdString();

    // Check if the project is valid using Project::checkProject
    if (Project::checkProject(folderPathStd)) {
        emit projectOpen(folderPathStd); // Emit signal if the project is valid
    } else {
        QMessageBox::critical(this, "Error", "The selected project is invalid. Please choose a valid project folder.");
    }
}