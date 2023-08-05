#include "FileOperationManager.h"
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QMessageBox>
#include <QDebug>

void FileOperationManager::executeFileOperation(const QStringList& fromPaths, const QString& toPath, const QString& operation) {
    QString fileOperationBinary = findFileOperationBinary();
    QProcess* process = new QProcess();

    QStringList arguments;
    arguments << operation;
    for (const QString& fromPath : fromPaths) {
        arguments << "'" + fromPath + "'";
    }
    arguments << "'" + toPath + "'";

    process->start(fileOperationBinary, arguments);

}

void FileOperationManager::copyWithProgress(const QStringList& fromPaths, const QString& toPath) {
    executeFileOperation(fromPaths, toPath, "--copy");
}

void FileOperationManager::moveWithProgress(const QStringList& fromPaths, const QString& toPath) {
    executeFileOperation(fromPaths, toPath, "--move");
}

QString FileOperationManager::findFileOperationBinary() {
    QStringList fileOperationBinaryCandidates;

    fileOperationBinaryCandidates << QCoreApplication::applicationDirPath() + QString("/bin/fileoperation")
                                  << QCoreApplication::applicationDirPath() + QString("/../../Resources/fileoperation")
                                  << QCoreApplication::applicationDirPath() + QString("/../bin/fileoperation")
                                  << QCoreApplication::applicationDirPath() + QString("/fileoperation/fileoperation");

    QString foundBinary;

    for (const QString &fileOperationBinaryCandidate : fileOperationBinaryCandidates) {
        if (QFile::exists(fileOperationBinaryCandidate) && QFileInfo(fileOperationBinaryCandidate).isExecutable()) {
            foundBinary = fileOperationBinaryCandidate;
            break;
        }
    }

    if (foundBinary.isEmpty()) {
        // Not found
        QMessageBox::critical(0, "Filer", "The 'fileoperation' command is missing. It should have been shipped with this application.");
    }

    return foundBinary;
}
