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

#include "VolumeWatcher.h"
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QStringList>
#include <QStorageInfo>
#include <QThread>
#include <QApplication>
#include <QProcess>
#include "AppGlobals.h"

VolumeWatcher::VolumeWatcher(QObject *parent) : QObject(parent)
{

    m_mediaPath = getMediaPath();

    m_watcher.addPath(m_mediaPath);

    // Run initially
    QString hardDiskName = tr("Hard Disk");
    if (! QFile::exists(QDir::homePath() + "/Desktop/" + AppGlobals::hardDiskName)) {
        QFile::link("/", QDir::homePath() + "/Desktop/" + AppGlobals::hardDiskName);
    }
    handleDirectoryChange(m_mediaPath);

    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, &VolumeWatcher::handleDirectoryChange);
    // We also need to get notified when directories are deleted, so we monitor the parent directory like this:
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &VolumeWatcher::handleDirectoryChange);
}

void VolumeWatcher::handleDirectoryChange(const QString &path)
{
    qDebug() << "Directory changed:" << path;
    QDir mediaDir(path);
    QStringList mediaDirectories = mediaDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    // Construct a list of symlinks that we have already created
    QString directoryPath = QDir::homePath() + "/Desktop"; // Change this to the directory you want to search

    QDir directory(directoryPath);
    QFileInfoList entries = directory.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs | QDir::System | QDir::Hidden | QDir::NoSymLinks, QDir::DirsFirst);

    QStringList symlinkPaths;

            foreach (const QFileInfo &entryInfo, entries) {
            if (entryInfo.isSymLink()) {
                // If links to /media
                if (entryInfo.symLinkTarget().startsWith(m_mediaPath + "/")) {
                    symlinkPaths.append(entryInfo.fileName());
                }
            }
        }

    qDebug() << "Symlink paths:" << symlinkPaths;

    for (const QString &dirName : mediaDirectories) {
        QString fullPath = mediaDir.absoluteFilePath(dirName);
        QString symlinkPath = QDir::homePath() + "/Desktop/" + dirName;

        if (QFile::exists(fullPath)) {
            if (!QFile::exists(symlinkPath)) {
                // Wait until a mount point appears at the target or 5 seconds have passed
                int counter = 0;
                QStringList mountPoints;
                while (true) {
                    for (const QStorageInfo &storage : QStorageInfo::mountedVolumes()) {
                        mountPoints << storage.rootPath();
                    }
                    if (mountPoints.contains(fullPath)) {
                        break;
                    }
                    QThread::msleep(100);
                    qApp->processEvents();
                    qDebug() << "Waiting for mount point to appear at" << fullPath;
                    // TODO: Replace this by an asynchronous approach
                    counter++;
                    if (counter > 10) {
                        // Give up after 1 second
                        break;
                    }
                }
                if (mountPoints.contains(fullPath)) {
                    QFile::link(fullPath, symlinkPath);
                    qDebug() << "Symlink created for" << fullPath;
                } else {
                    qDebug() << "Giving up on" << fullPath;
                }
            } else {
                qDebug() << "Symlink already exists for" << fullPath;
            }
        }
    }

    // Clean up symlinks for targets that no longer exist in /media/
    for (const QString &symlinkPath : symlinkPaths) {
        QString fullPath = m_mediaPath + "/" + symlinkPath;
        if (!QFile::exists(fullPath)) {
            QFile::remove(QDir::homePath() + "/Desktop/" + symlinkPath);
            qDebug() << "Symlink removed for" << fullPath;
        }
    }
}

QString VolumeWatcher::getMediaPath() {
    QString mediaPath;
    // Check whether a process "udisksd" is running; if yes, we assume that /media/$USER is managed by udisksd
    // and should be used instead of /media which is managed e.g., by /usr/local/sbin/automount on FreeBSD
    // pidof udisksd
    QProcess pidof;
    pidof.start("pidof udisksd");
    pidof.waitForFinished();
    if (pidof.exitCode() == 0) {
        qDebug() << "Using /media/$USER since udisksd is running";
        QString candidate = "/media/" + qgetenv("USER");
        QDir().mkdir(mediaPath);
        if (QDir(mediaPath).exists()) {
            mediaPath = candidate;
        } else {
            // We cannot use /media/$USER, so we fall back to /media
            qDebug() << "Using /media";
            mediaPath = "/media";
        }
    } else {
        qDebug() << "Using /media";
        mediaPath = "/media";
    }
    return mediaPath;
}
