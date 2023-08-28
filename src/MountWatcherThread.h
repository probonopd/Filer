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

#ifndef MOUNTWATCHERTHREAD_H
#define MOUNTWATCHERTHREAD_H

#include <QThread>

/**
 * @brief The MountWatcherThread class provides a threaded mechanism for
 *        watching and creating symlinks for mounted volumes. This class exists to make
 *        waiting for the mounting to happen asynchronous, not blocking everything else.
 *        QStorageInfo doesn't provide a built-in mechanism to notify when mounted volumes change;
 *        if it did, we would not need this.
 */
class MountWatcherThread : public QThread {
Q_OBJECT

public:
    /**
     * @brief Constructs a MountWatcherThread object.
     * @param fullPath The full path of the mount point.
     * @param symlinkPath The path where the symlink should be created.
     */
    MountWatcherThread(const QString &fullPath, const QString &symlinkPath);

protected:
    /**
     * @brief The overridden run method that performs the async logic for
     *        watching and creating symlinks.
     */
    void run() override;

private:
    QString fullPath; ///< The full path of the mount point.
    QString symlinkPath; ///< The path where the symlink should be created.
};

#endif // MOUNTWATCHERTHREAD_H
