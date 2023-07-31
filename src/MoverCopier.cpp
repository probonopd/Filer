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

#include "MoverCopier.h"
#include <QMessageBox>

// Custom exception class for file operation errors
class FileOperationException : public std::exception
{
public:
    FileOperationException(const QString &message) : errorMessage(message.toStdString()) {}

    const char *what() const noexcept override
    {
        return errorMessage.c_str();
    }

private:
    std::string errorMessage;
};


MoverCopier::MoverCopier(QObject *parent) : QObject(parent), isCanceled(false)
{
    progressDialog.setLabelText(tr("Moving/copying files..."));
    progressDialog.setMinimumDuration(0);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setValue(0);

    connect(&progressDialog, &QProgressDialog::canceled, this, &MoverCopier::cancel);
}

void MoverCopier::copy(const QString &source, const QString &destination, bool recursive)
{
    isCanceled = false;
    qint64 totalSize = getTotalSize(source, recursive);
    progressDialog.setMaximum(totalSize);

    try {
        copyFiles(source, destination, recursive);
        progressDialog.close();
    } catch (const FileOperationException &e) {
        progressDialog.close();
        showErrorMessage(tr("Copy Error"), QString::fromStdString(e.what()));
    }
}

void MoverCopier::move(const QString &source, const QString &destination, bool recursive)
{
    isCanceled = false;
    qint64 totalSize = getTotalSize(source, recursive);
    progressDialog.setMaximum(totalSize);

    try {
        moveFiles(source, destination, recursive);
        progressDialog.close();
    } catch (const FileOperationException &e) {
        progressDialog.close();
        showErrorMessage(tr("Move Error"), QString::fromStdString(e.what()));
    }
}

void MoverCopier::cancel()
{
    isCanceled = true;
}

qint64 MoverCopier::getTotalSize(const QString &source, bool recursive)
{
    qint64 totalSize = 0;

    // Get the list of files and directories in the source
    QDir dir(source);
    QFileInfoList entries =
            dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs | QDir::NoSymLinks);

    // Iterate over the entries
            foreach (const QFileInfo &entry, entries) {
            // If the entry is a file, add its size to the total
            if (entry.isFile()) {
                totalSize += entry.size();
            }

            // If the entry is a directory and recursive copying/moving is enabled,
            // recursively get the total size of the files in the directory
            if (entry.isDir() && recursive) {
                totalSize += getTotalSize(entry.absoluteFilePath(), recursive);
            }
        }

    return totalSize;
}

void MoverCopier::copyFiles(const QString &source, const QString &destination, bool recursive)
{
    // Get the list of files and directories in the source
    QDir dir(source);
    QFileInfoList entries =
            dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs | QDir::NoSymLinks);

    // Create the destination directory if it doesn't exist
    if (!dir.exists(destination)) {
        dir.mkpath(destination);
    }

    // Iterate over the entries
            foreach (const QFileInfo &entry, entries) {
            // Check if the operation has been canceled
            if (isCanceled) {
                return;
            }

            // If the entry is a file, copy it to the destination
            if (entry.isFile()) {
                QFile::copy(entry.absoluteFilePath(),
                            destination + QDir::separator() + entry.fileName());

                // Update the progress
                updateProgress(entry.size());
            }

            // If the entry is a directory and recursive copying is enabled,
            // recursively copy the files in the directory
            if (entry.isDir() && recursive) {
                copyFiles(entry.absoluteFilePath(), destination + QDir::separator() + entry.fileName(),
                          recursive);
            }
        }
}

void MoverCopier::moveFiles(const QString &source, const QString &destination, bool recursive)
{
    // Get the list of files and directories in the source
    QDir dir(source);
    QFileInfoList entries =
            dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs | QDir::NoSymLinks);

    // Create the destination directory if it doesn't exist
    if (!dir.exists(destination)) {
        dir.mkpath(destination);
    }

    // Iterate over the entries
            foreach (const QFileInfo &entry, entries) {
            // Check if the operation has been canceled
            if (isCanceled) {
                return;
            }

            // If the entry is a file, move it to the destination
            if (entry.isFile()) {
                QFile::rename(entry.absoluteFilePath(),
                              destination + QDir::separator() + entry.fileName());

                // Update the progress
                updateProgress(entry.size());
            }

            // If the entry is a directory and recursive moving is enabled,
            // recursively move the files in the directory
            if (entry.isDir() && recursive) {
                moveFiles(entry.absoluteFilePath(), destination + QDir::separator() + entry.fileName(),
                          recursive);
            }
        }
}

void MoverCopier::updateProgress(qint64 value)
{
    progressDialog.setValue(progressDialog.value() + value);
}

void MoverCopier::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Period && event->modifiers() == Qt::ControlModifier) {
        cancel();
    }
}

void MoverCopier::stopOperation()
{
    isCanceled = true;
}

void MoverCopier::showErrorMessage(const QString &title, const QString &message)
{
    QMessageBox::critical(nullptr, title, message, QMessageBox::Ok);
}