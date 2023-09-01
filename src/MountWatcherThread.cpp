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

#include "MountWatcherThread.h"

#include <QFile>
#include <QStorageInfo>
#include <QDebug>
#include <QThread>

MountWatcherThread::MountWatcherThread(const QString &fullPath, const QString &symlinkPath)
        : fullPath(fullPath), symlinkPath(symlinkPath) {}

void MountWatcherThread::run() {
    if (QFile::exists(fullPath)) {
        if (!QFile::exists(symlinkPath)) {
            int counter = 0;
            QStringList mountPoints;
            while (true) {
                for (const QStorageInfo &storage : QStorageInfo::mountedVolumes()) {
                    mountPoints << storage.rootPath();
                }
                if (mountPoints.contains(fullPath)) {
                    QFile::link(fullPath, symlinkPath);
                    qDebug() << "Symlink created for" << fullPath;
                    break;
                }
                QThread::msleep(100);
                qDebug() << "Waiting for mount point to appear at" << fullPath;
                counter++;
                // Give up after 1 minute
                if (counter > 600) {
                    qDebug() << "Giving up on" << fullPath;
                    // Delete the symlink if it exists
                    if (QFile::exists(symlinkPath)) {
                        QFile::remove(symlinkPath);
                    }
                    // Delete the source if it exists
                    if (QFile::exists(fullPath)) {
                        // The directory must be empty for rmdir() to succeed.
                        QDir::root().rmdir(fullPath);
                    }
                    break;
                }
            }
        } else {
            qDebug() << "Symlink already exists for" << fullPath;
        }
    }
}
