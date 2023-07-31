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

/* The MoverCopier class provides two public methods:
 * copy() and move() which can be used to copy or move a file or a directory from one location to
 * another. The copy() and move() methods take three arguments: The source file or directory path.
 * The destination file or directory path.
 * A boolean flag indicating whether to recursively copy or move subdirectories (defaults to true).
 * Internally, the MoverCopier class uses the copyFile(), copyDirectory(), moveFile(), and
 * moveDirectory() methods to perform the actual copying or moving of files and directories. These
 * methods also update a QProgressDialog instance to show the progress and estimated time remaining
 * for the operation. A QTimer instance is also used to periodically reset the progress dialog to
 * update the estimated time remaining.
 */

#ifndef MOVERCOPIER_H
#define MOVERCOPIER_H

#include <QObject>
#include <QString>
#include <QIODevice>
#include <QProgressDialog>
#include <QKeyEvent>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

class MoverCopier : public QObject
{
    Q_OBJECT

public:
    explicit MoverCopier(QObject *parent = nullptr);

    // Function to copy files and directories
    void copy(const QString &source, const QString &destination, bool recursive);

    // Function to move files and directories
    void move(const QString &source, const QString &destination, bool recursive);

public slots:
    // Function to cancel the ongoing operation
    void cancel();

signals:
    // Signal emitted when the operation is canceled
    void canceled();

private:
    // Helper function to get the total size of files in a directory (including subdirectories)
    qint64 getTotalSize(const QString &source, bool recursive);

    // Helper function to copy files and directories recursively
    void copyFiles(const QString &source, const QString &destination, bool recursive);

    // Helper function to move files and directories recursively
    void moveFiles(const QString &source, const QString &destination, bool recursive);

    // Progress dialog to show the progress of the operation
    QProgressDialog progressDialog;

    // Flag to indicate if the operation is canceled
    bool isCanceled = false;

    // Function to update the progress dialog value
    void updateProgress(qint64 value);

    // Function to handle key press events, used for cancelling the operation
    void keyPressEvent(QKeyEvent *event);

    // Function to stop the current operation
    void stopOperation();

    void showErrorMessage(const QString &title, const QString &message);
};

#endif // MOVERCOPIER_H
