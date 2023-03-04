/*-
 * Copyright (c) 2022 Simon Peter <probono@puredarwin.org>
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

#include "movercopier.h"

#include <QObject>
#include <QString>
#include <QFile>
#include <QDir>
#include <QProgressDialog>
#include <QTimer>
#include <QDebug>
#include <QPushButton>
#include <QKeyEvent>

#include "movercopier.h"

MoverCopier::MoverCopier(QObject *parent) : QObject(parent)
{
    // Create a progress dialog with a cancel button
    progressDialog = new QProgressDialog;
    progressDialog->setLabelText(tr("Moving/copying files..."));
    progressDialog->setMinimumDuration(0); // Show the dialog immediately
    progressDialog->setWindowModality(Qt::WindowModal); // Block input to other windows
    progressDialog->setValue(0); // Initialize the progress to 0

    // Connect the cancel button to the cancel slot
    connect(progressDialog, SIGNAL(canceled()), this, SLOT(cancel()));
}

void MoverCopier::copy(const QString &source, const QString &destination, bool recursive)
{
    // Get the total size of all files in the source directory
    qint64 totalSize = getTotalSize(source, recursive);
    progressDialog->setMaximum(totalSize); // Set the maximum value of the progress indicator

    // Copy the files
    copyFiles(source, destination, recursive);

    // Close the progress dialog when the operation is complete
    progressDialog->close();
}

void MoverCopier::move(const QString &source, const QString &destination, bool recursive)
{
    // Get the total size of all files in the source directory
    qint64 totalSize = getTotalSize(source, recursive);
    progressDialog->setMaximum(totalSize); // Set the maximum value of the progress indicator

    // Move the files
    moveFiles(source, destination, recursive);

    // Close the progress dialog when the operation is complete
    progressDialog->close();
}

void MoverCopier::cancel()
{
    // Stop the current operation
    stopOperation();

    // Close the progress dialog
    progressDialog->close();
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
        // If the entry is a file, copy it to the destination
        if (entry.isFile()) {
            QFile::copy(entry.absoluteFilePath(),
                        destination + QDir::separator() + entry.fileName());

            // Update the progress
            progressDialog->setValue(progressDialog->value() + entry.size());
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
        // If the entry is a file, move it to the destination
        if (entry.isFile()) {
            QFile::rename(entry.absoluteFilePath(),
                          destination + QDir::separator() + entry.fileName());

            // Update the progress
            progressDialog->setValue(progressDialog->value() + entry.size());
        }

        // If the entry is a directory and recursive moving is enabled,
        // recursively move the files in the directory
        if (entry.isDir() && recursive) {
            moveFiles(entry.absoluteFilePath(), destination + QDir::separator() + entry.fileName(),
                      recursive);
        }
    }
}

// Set Shortcut Command+. to cancel the current operation
void MoverCopier::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Period && event->modifiers() == Qt::ControlModifier) {
        cancel();
    }
}

void MoverCopier::stopOperation()
{
    cancel();
}
