/*-
 * Copyright (c) 2022-23 Simon Peter <probono@puredarwin.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "TrashHandler.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QStorageInfo>
#include <QDebug>
#include <QProcess>
#include <QMessageBox>
#include "SoundPlayer.h"
#include <QTimer>
#include "FileManagerMainWindow.h"
#include <QThread>
#include "AppGlobals.h"
#include <QFileSystemWatcher>

QString TrashHandler::m_trashPath = QDir::homePath() + "/.local/share/Trash/files";

TrashHandler::TrashHandler(QWidget *parent) : QObject(parent) {
    m_parent = parent;
    m_dialogShown = false;

    // Tell the application to reload the desktop whenever
    // the filesystem at TrashHandler::getTrashPath() changes
    QFileSystemWatcher *fileSystemWatcher = new QFileSystemWatcher(this);
    fileSystemWatcher->addPath(TrashHandler::getTrashPath());
    connect(fileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, [=]() {
        qDebug() << "TrashHandler::trashChanged";
        // Reload the desktop
        QProcess p;
        p.start("touch", QStringList() << QDir::homePath() + QDir::separator() + "Desktop");
        p.waitForFinished(-1);
    });
}

void TrashHandler::moveToTrash(const QStringList& paths) {

    qDebug() << "moveToTrash" << paths;

    // This is used to know which sound to play at the end
    bool unmounted = false;
    bool filesMoved = false;

    foreach (const QString &path, paths) {

        // Be conservative and assume that the path is a mount point
        // until we have proven otherwise
        bool isMountpoint = true;

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

        if (! mountPoints.contains(absoluteFilePathWithSymlinksResolved)) {
            isMountpoint = false;
        }

        if (isMountpoint) {
            qDebug() << "Path" << absoluteFilePathWithSymlinksResolved << "is a mount point";

            // Check if there is a window for the mount point open and if so, close it
            // Get the list of open windows from FileManagerMainWindow
            FileManagerMainWindow* mainWindow = qobject_cast<FileManagerMainWindow*>(qApp->activeWindow());

            if (mainWindow->instanceExists(absoluteFilePathWithSymlinksResolved)) {
                // Close the window for the mount point
                qDebug() << "Closing window for mount point" << absoluteFilePathWithSymlinksResolved;
                FileManagerMainWindow* targetWindow = mainWindow->getInstanceForDirectory(absoluteFilePathWithSymlinksResolved);
                if (targetWindow != nullptr) {
                    targetWindow->close();
                    // Process events to make sure the window is closed; for one second; FIXME: This is a hack and does not work
                    // for (int i = 0; i < 200; i++) {
                    //     QCoreApplication::processEvents();
                    //     QThread::msleep(10);
                    // }
                }
            }

            // Unmount the mount point
            // TODO: Might be necessary to call with sudo -A -E
            QProcess umount;
            // If eject-and-clean exists, use it; otherwise use umount
            // eject-and-clean is a wrapper around umount that also cleans up the mount point
            if (QFile::exists("/usr/local/bin/eject-and-clean")) {
                umount.start("eject-and-clean", QStringList() << absoluteFilePathWithSymlinksResolved);
                qDebug() << "eject-and-clean" << absoluteFilePathWithSymlinksResolved;
            } else {
                umount.start("umount", QStringList() << absoluteFilePathWithSymlinksResolved);
                qDebug() << "umount" << absoluteFilePathWithSymlinksResolved;
            }
            qDebug() << "Waiting for umount to finish";
            umount.waitForFinished(10000); // Wait for 10 seconds; it can really take that long...
            if (umount.exitCode() == 0) {
                unmounted = true;
            } else {
                QMessageBox::critical(nullptr, tr("Error"),
                                      tr("Failed to unmount the mount point: ") + absoluteFilePathWithSymlinksResolved);
            }
            continue;
        }

        if (isMountpoint) {
            qDebug() << "Path" << absoluteFilePathWithSymlinksResolved << "is a mount point, skipping";
            continue;
        }

        // If path is /media or /media/$USER, we refuse to move it to the trash
        if (absoluteFilePathWithSymlinksResolved == AppGlobals::mediaPath) {
            qDebug() << "Path" << absoluteFilePathWithSymlinksResolved << "is in" << AppGlobals::mediaPath << ", skipping";
            continue;
        }

        if (!fileInfo.exists()) {
            QMessageBox::warning(nullptr, tr("File not found"),
                                 tr("The file or directory does not exist."));
            continue;
        }

        QDir trashDir(m_trashPath);

        // Create the "Trash" directory if it doesn't exist
        if (!trashDir.exists()) {
            if (!trashDir.mkpath(".")) {
                QMessageBox::critical(nullptr, tr("Error"),
                                      tr("Failed to create the Trash directory."));
                continue;
            }
        }

        QString fileName = fileInfo.fileName();
        QString newFilePath = m_trashPath + QDir::separator() + fileName;

        // Check if the file/directory is a critical system file/directory
        QStringList criticalSystemPaths = {"/",
                                           "/Applications",
                                           "/COPYRIGHT",
                                           "/System",
                                           "/Users",
                                           "/bin",
                                           "/boot",
                                           "/compat",
                                           "/dev",
                                           "/entropy",
                                           "/etc",
                                           "/home",
                                           "/lib",
                                           "/libexec",
                                           "/media",
                                           "/mnt",
                                           "/net",
                                           "/proc",
                                           "/rescue",
                                           "/root",
                                           "/sbin",
                                           "/sys",
                                           "/tmp",
                                           "/usr",
                                           "/usr/bin",
                                           "/usr/home",
                                            "/usr/lib",
                                            "/usr/libexec",
                                            "/usr/local",
                                            "/usr/local/bin",
                                            "/usr/local/etc",
                                            "/usr/local/games",
                                            "/usr/local/include",
                                            "/usr/local/lib",
                                            "/usr/local/libexec",
                                            "/usr/local/sbin",
                                            "/usr/local/share",
                                            "/usr/local/src",
                                            "/usr/obj",
                                            "/usr/ports",
                                            "/usr/sbin",
                                            "/usr/share",
                                            "/usr/src",
                                           "/var",
                                           "/zroot"};

        if (criticalSystemPaths.contains(absoluteFilePathWithSymlinksResolved)) {
            QMessageBox::critical(nullptr, tr("Error"),
                                  tr("This is critical for the system and cannot be moved to the trash."));
            continue;
        }

        // Check if the file/directory with the same name already exists in the Trash
        int i = 1;
        while (QFile::exists(newFilePath)) {
            QString newFileName = fileInfo.baseName() + QString("_%1").arg(i++) + "." + fileInfo.suffix();
            newFilePath = m_trashPath + QDir::separator() + newFileName;
        }

        if (! m_dialogShown) {
            // Show a confirmation dialog (once)
            m_dialogShown = true;
            int result = QMessageBox::warning(m_parent, tr("Confirm"),
                                              tr("Do you want to move the selected items to the Trash?"),
                                              QMessageBox::Yes | QMessageBox::No,
                                              QMessageBox::No);
            if (result != QMessageBox::Yes) {
                return;
            }
        }

        // Check if the file/directory is on the same mount point as the Trash directory
        QFileInfo trashDirInfo(m_trashPath);
        if (trashDirInfo.isDir()) {
            QStorageInfo trashDirStorageInfo(m_trashPath);
            QStorageInfo fileStorageInfo(path);
            qDebug() << "Trash dir mount point: " << trashDirStorageInfo.rootPath();
            qDebug() << "File root mount point: " << fileStorageInfo.rootPath();
            if (trashDirStorageInfo.rootPath() == fileStorageInfo.rootPath()) {
                // The file/directory is on the same mount point as the Trash directory
                // Move the file/directory to the Trash directory
                qDebug() << "The file/directory is on the same mount point as the Trash directory";
                if (!QFile::rename(path, newFilePath)) {
                    // Failed to move the file/directory to Trash
                    // Restore the original file back to its original location
                    QFile::rename(newFilePath, path);

                    QMessageBox::critical(nullptr, tr("Error"),
                                          tr("Failed to move to Trash. Please check file permissions."));
                    continue;
                } else {
                    filesMoved = true;
                    continue;
                }
            } else {
                // The file/directory is on a different mount point than the Trash directory, hence
                // inform the user and as whether to delete the file/directory permanently right away
                qDebug() << "The file/directory is on a different mount point than the Trash directory";
                int result = QMessageBox::warning(m_parent, tr("Confirm"),
                                                  tr("The selected items are on a different mount point than the Trash directory. "
                                                     "Do you want to delete the selected items permanently right away?"),
                                                  QMessageBox::Yes | QMessageBox::No,
                                                  QMessageBox::No);
                if (result == QMessageBox::Yes) {
                    // Delete the file/directory permanently
                    if (fileInfo.isDir()) {
                        if (!QDir(path).removeRecursively()) {
                            // Use sudo -A -E rm -rf <path> to delete the directory
                            QProcess p;
                            p.start("sudo", QStringList() << "-A" << "-E" << "rm" << "-rf" << path);
                            p.waitForFinished(-1);
                            if (p.exitCode() != 0) {
                                QMessageBox::critical(nullptr, tr("Error"),
                                                      tr("Failed to delete the directory permanently. Please check file permissions."));
                            }
                            continue;
                        }
                    } else {
                        if (!QFile::remove(path)) {
                            // Use sudo -A -E rm -f <path> to delete the file
                            QProcess p;
                            p.start("sudo", QStringList() << "-A" << "-E" << "rm" << "-f" << path);
                            p.waitForFinished(-1);
                            if (p.exitCode() != 0) {
                                QMessageBox::critical(nullptr, tr("Error"),
                                                      tr("Failed to delete the file permanently. Please check file permissions."));
                            }
                            continue;
                        }
                    }
                } else {
                    // Do not delete the file/directory permanently
                    continue;
                }
            }
        }
    }

    if (unmounted && filesMoved) {
        // Both mount points were unmounted and files were moved to trash
        SoundPlayer::playSound("ffft.wav");
    } else if (unmounted) {
        // Only mount points were unmounted
        SoundPlayer::playSound("pschiuu.wav");
    } else if (filesMoved) {
        // Only files were moved to trash
        SoundPlayer::playSound("ffft.wav");
    }
}

bool TrashHandler::emptyTrash() {
    QDir trashDir(m_trashPath);

    // Ask user for confirmation
    int result = QMessageBox::warning(nullptr, tr("Trash"),
                                      tr("Do you want to permanently delete all items in the Trash?"),
                                      QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::No);
    if (result != QMessageBox::Yes) {
        return false;
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

    SoundPlayer::playSound("rustle.wav");

    // Tell the main window to update its menus because there is now something in the Trash
    // so the "Empty Trash" menu item should be enabled
    FileManagerMainWindow* mainWindow = qobject_cast<FileManagerMainWindow*>(qApp->activeWindow());
    mainWindow->updateMenus();

    // Use QProcess to run the following command: "touch ~/Desktop"
    // this will cause the Trash icon on the Desktop to be refreshed, so that
    // it will show the empty Trash icon.
    QProcess p;
    p.start("touch", QStringList() << QDir::homePath() + QDir::separator() + "Desktop");
    p.waitForFinished(-1);

    return true;
}

QString TrashHandler::getTrashPath() {
    return m_trashPath;
}

bool TrashHandler::isEmpty() {
    QDir trashDir(m_trashPath);
    return trashDir.isEmpty();
}