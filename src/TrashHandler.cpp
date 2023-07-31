#include "TrashHandler.h"
#include <QCoreApplication>
#include <QLocale>
#include <QTranslator>
#include <QStorageInfo>
#include <QDebug>
#include <QProcess>

TrashHandler::TrashHandler(QObject *parent) : QObject(parent) {
    m_trashPath = QDir::homePath() + "/.local/share/Trash/files";
}

bool TrashHandler::moveToTrash(const QString& path) {
    QFileInfo fileInfo(path);

    // Check if the path is a mount point using QStorageInfo
    QStringList mountPoints;
    for (const QStorageInfo &storage : QStorageInfo::mountedVolumes()) {
        mountPoints << storage.rootPath();
    }

    QString absoluteFilePathWithSymlinksResolved = fileInfo.absoluteFilePath();
    if (fileInfo.isSymLink()) {
        absoluteFilePathWithSymlinksResolved = fileInfo.symLinkTarget();
    }

    if (mountPoints.contains(absoluteFilePathWithSymlinksResolved)) {
        // Unmount the mount point
        // TODO: Might be necessary to call with sudo -A -E
        QProcess umount;
        umount.start("umount", QStringList() << absoluteFilePathWithSymlinksResolved);
        umount.waitForFinished(-1);
        if (umount.exitCode() == 0) {
            // Successfully unmounted the mount point, now remove the mount point
            QDir mountPointDir(absoluteFilePathWithSymlinksResolved);
            // Remove the mount point directory but not recursively
            if (!mountPointDir.rmdir(".")) {
                return false;
            }
        }
        return true;
    }

    if (!fileInfo.exists()) {
        QMessageBox::warning(nullptr, tr("File not found"),
                             tr("The file or directory does not exist."));
        return false;
    }

    QDir trashDir(m_trashPath);

    // Create the "Trash" directory if it doesn't exist
    if (!trashDir.exists()) {
        if (!trashDir.mkpath(".")) {
            QMessageBox::critical(nullptr, tr("Error"),
                                  tr("Failed to create the Trash directory."));
            return false;
        }
    }

    QString fileName = fileInfo.fileName();
    QString newFilePath = m_trashPath + QDir::separator() + fileName;

    // Check if the file/directory with the same name already exists in the Trash
    int i = 1;
    while (QFile::exists(newFilePath)) {
        QString newFileName = fileInfo.baseName() + QString("_%1").arg(i++) + "." + fileInfo.suffix();
        newFilePath = m_trashPath + QDir::separator() + newFileName;
    }

    // Move the file/directory to the Trash directory
    if (!QFile::rename(path, newFilePath)) {
        // Failed to move the file/directory to Trash
        // Restore the original file back to its original location
        QFile::rename(newFilePath, path);

        QMessageBox::critical(nullptr, tr("Error"),
                         tr("Failed to move to Trash. Please check file permissions."));
        return false;
    }

    return true;
}

bool TrashHandler::emptyTrash() {
    QDir trashDir(m_trashPath);

    if (!trashDir.exists()) {
        QMessageBox::information(nullptr, tr("Empty Trash"),
                                 tr("Trash is already empty."));
        return true;
    }

    // Remove all files and directories from the Trash directory
    QStringList trashItems = trashDir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries);
    for (const QString& trashItem : trashItems) {
        QString trashPath = m_trashPath + QDir::separator() + trashItem;

        if (QFileInfo(trashPath).isDir()) {
            // Recursively remove subdirectories and files
            QDir subDir(trashPath);
            if (!subDir.removeRecursively()) {
                QMessageBox::critical(nullptr, tr("Error"),
                                      tr("Failed to remove directory from Trash: %1").arg(trashItem));
                return false;
            }
        } else {
            // Remove the file
            if (!QFile::remove(trashPath)) {
                QMessageBox::critical(nullptr, tr("Error"),
                                      tr("Failed to remove file from Trash: %1").arg(trashItem));
                return false;
            }
        }
    }

    // Remove the Trash directory itself
    if (!trashDir.rmdir(".")) {
        QMessageBox::critical(nullptr, tr("Error"),
                              tr("Failed to remove the Trash directory."));
        return false;
    }

    QMessageBox::information(nullptr, tr("Empty Trash"),
                             tr("Trash has been emptied successfully."));
    return true;
}

QString TrashHandler::getTrashPath() const {
    return m_trashPath;
}
