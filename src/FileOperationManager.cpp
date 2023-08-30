#include "FileOperationManager.h"
#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QMessageBox>
#include <QDebug>
#include <QDir>

void FileOperationManager::executeFileOperation(const QStringList& fromPaths, const QString& toPath, const QString& operation) {

    if(operation != "--copy" && operation != "--move") {
        QMessageBox::critical(0, "Filer", "Invalid operation.");
        return;
    }

    // Check if the source and destination trees are accessible by the current user;
    // if not, the operation needs root privileges.
    // TODO: We might also want to check whether the trees are on read-only media, in which case
    // not even root can write to them.
    bool operationNeedsRoot = false;
    if (!areTreesAccessible(QStringList(toPath), Writeable)) {
        operationNeedsRoot = true;
    }
    if(operation == "--copy") {
        if (!areTreesAccessible(fromPaths, Readable)) {
            operationNeedsRoot = true;
        }
    } else if(operation == "--move") {
        if (!areTreesAccessible(fromPaths, Writeable)) {
            operationNeedsRoot = true;
        }
    } else {
        qDebug() << "Invalid operation.";
        return;
    }

    QString fileOperationBinary = findFileOperationBinary();
    QProcess* process = new QProcess();

    QStringList arguments;
    arguments << operation;
    for (const QString& fromPath : fromPaths) {
        arguments << fromPath;
    }
    arguments << toPath;

    if (operationNeedsRoot) {
        /*
        // Ask user whether to run the operation as root
        QMessageBox::StandardButton reply;
        // FIXME: Why does tr() not work here?
        reply = QMessageBox::question(0, 0, "This operation requires root privileges. Do you want to continue?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No) {
            return;
        }
        */
        process->setProgram("sudo");
        QStringList sudoArguments;
        sudoArguments << "-A" << "-E" << fileOperationBinary << arguments;
        process->setArguments(sudoArguments);
        qDebug() << "Executing file operation:" << sudoArguments;
        process->start();
    } else {
        process->setProgram(fileOperationBinary);
        process->setArguments(arguments);
        qDebug() << "Executing file operation:" << fileOperationBinary << arguments;
        process->start(fileOperationBinary, arguments);
    }
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

bool FileOperationManager::areTreesAccessible(const QStringList &paths, AccessType accessType) {
    for (const QString &path : paths) {
        QDir rootDir(path);
        if (accessType == Writeable) {
            if (!isTreeWritable(rootDir)) {
                return false;
            }
        } else if (accessType == Readable) {
            if (!isTreeReadable(rootDir)) {
                return false;
            }
        }
    }
    return true;
}

bool FileOperationManager::isTreeWritable(const QDir &dir) {
    qDebug() << "Checking if tree is writable:" << dir.absolutePath();
    QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs | QDir::Hidden);

    for (const QFileInfo &entry : entries) {
        if (!entry.isWritable()) {
            qDebug() << "Entry is not writable:" << entry.filePath();
            return false;
        }

        if (entry.isDir()) {
            QDir subDir(entry.filePath());
            if (!isTreeWritable(subDir)) {
                qDebug() << "Subtree is not writable:" << subDir.absolutePath();
                return false;
            }
        }
    }
    qDebug() << "Tree is writable:" << dir.absolutePath();
    return true;
}

bool FileOperationManager::isTreeReadable(const QDir &dir) {
    qDebug() << "Checking if tree is readable:" << dir.absolutePath();
    QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs | QDir::Hidden);

    for (const QFileInfo &entry : entries) {
        if (!entry.isReadable()) {
            qDebug() << "Entry is not readable:" << entry.filePath();
            return false;
        }

        if (entry.isDir()) {
            QDir subDir(entry.filePath());
            if (!isTreeReadable(subDir)) {
                qDebug() << "Subtree is not readable:" << subDir.absolutePath();
                return false;
            }
        }
    }
    qDebug() << "Tree is readable:" << dir.absolutePath();
    return true;
}