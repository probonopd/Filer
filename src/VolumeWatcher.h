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

#ifndef VOLUMEWATCHER_H
#define VOLUMEWATCHER_H

#include <QObject>
#include <QFileSystemWatcher>

/**
 * @file VolumeWatcher.h
 * @class VolumeWatcher
 * @brief The VolumeWatcher class monitors changes in a directory and manages symlinks to new directories.
 *
 * This class uses QFileSystemWatcher to keep track of changes in the /media directory.
 * When a new subdirectory appears, it creates a symlink to that subdirectory on the user's desktop.
 * If a subdirectory disappears, the corresponding symlink is removed.
 * @Note This class should be replaced by a more appropriate solution, e.g., using a QProxyModel
 * to display the contents of the /media directory alongside the contents of the user's home directory
 * without the need to create symlinks.
 */
class VolumeWatcher : public QObject
{
Q_OBJECT

public:
    /**
     * @brief Constructs a VolumeWatcher object.
     * @param parent The parent QObject.
     */
    explicit VolumeWatcher(QObject *parent = nullptr);

    static QString getMediaPath();

    static QString getRootDiskName();

private slots:
    /**
     * @brief Handles changes in the monitored directory.
     * @param path The path of the directory that has changed.
     */
    void handleDirectoryChange(const QString &path);

private:
    QFileSystemWatcher m_watcher; /**< The QFileSystemWatcher used to monitor changes. */
    QString m_mediaPath; /**< The path of the directory to monitor. */
    void startWaitingForMounted(const QString &fullPath, const QString &symlinkPath) const;
};

#endif // VOLUMEWATCHER_H
