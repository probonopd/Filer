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
#include <QCoreApplication>
#include <QLocale>
#include <QTranslator>
#include <QStorageInfo>
#include <QDebug>
#include <QProcess>
#include <QMessageBox>
#include "SoundPlayer.h"

TrashHandler::TrashHandler(QWidget *parent) : QObject(parent) {
    m_trashPath = QDir::homePath() + "/.local/share/Trash/files";
    m_parent = parent;
    m_dialogShown = false;
}

void TrashHandler::moveToTrash(const QStringList& paths) {

    // This is used to know which sound to play at the end
    bool unmounted = false;
    bool filesMoved = false;

    foreach (const QString &path, paths) {

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
            umount.waitForFinished(10000);
            if (umount.exitCode() == 0) {
                // Successfully unmounted the mount point, now remove the mount point
                QDir mountPointDir(absoluteFilePathWithSymlinksResolved);
                // Remove the mount point directory but not recursively
                if (!mountPointDir.rmdir(".")) {
                    QMessageBox::critical(nullptr, tr("Error"),
                                          tr("Failed to remove the mount point directory: ") + absoluteFilePathWithSymlinksResolved);
                }
                unmounted = true;
                continue;
            } else {
                QMessageBox::critical(nullptr, tr("Error"),
                                      tr("Failed to unmount") + absoluteFilePathWithSymlinksResolved);
                continue;
            }
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

        // Move the file/directory to the Trash directory
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
    }
    SoundPlayer::playSound("ffft.wav");

    if (unmounted && filesMoved) {
        // Both mount points were unmounted and files were moved to trash
        SoundPlayer::playSound("ffft.wav");
    } else if (unmounted) {
        // Only mount points were unmounted
        SoundPlayer::playSound("trashmount.wav");
    } else if (filesMoved) {
        // Only files were moved to trash
        SoundPlayer::playSound("ffft.wav");
    }
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

    SoundPlayer::playSound("rustle.wav");

    QMessageBox::information(nullptr, tr("Empty Trash"),
                             tr("Trash has been emptied successfully."));
    return true;
}

QString TrashHandler::getTrashPath() const {
    return m_trashPath;
}
