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

#include "DBusInterface.h"

#include <QDebug>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QMessageBox>
#include <QApplication>
#include <QUrl>
#include <QFileInfo>

#include "FileManagerMainWindow.h"
#include "InfoDialog.h"

DBusInterface::DBusInterface()
    : QObject()
{
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/org/freedesktop/FileManager1"),
         this,
         QDBusConnection::ExportScriptableContents | QDBusConnection::ExportAdaptors);
    QDBusConnectionInterface *sessionInterface = QDBusConnection::sessionBus().interface();
    if (sessionInterface) {
        sessionInterface->registerService(QStringLiteral("org.freedesktop.FileManager1"),
                                          QDBusConnectionInterface::QueueService);
    }

    if (!QDBusConnection::sessionBus().interface()) {
        QMessageBox::warning(0, 0, tr("Could not register the FileManager1 D-Bus service"));
    }
}

void DBusInterface::ShowFolders(const QStringList &uriList, const QString &startUpId)
{
    qDebug() << "ShowFolders" << uriList << startUpId;

    for (const QString &fileUrlString : uriList) {
        QUrl fileUrl(fileUrlString);
        QString filePath;

        if (fileUrl.isValid()) {
            filePath = fileUrl.toLocalFile();
        } else {
            if (QFileInfo::exists(fileUrlString)) {
                filePath = fileUrlString;
            } else {
                qDebug() << "Not a URL or a local file path:" << fileUrlString;
                QMessageBox::warning(0, QString(), tr("Not a URL or a local file path: %1").arg(fileUrlString));
                continue;
            }
        }

        FileManagerMainWindow *mainWindow = qobject_cast<FileManagerMainWindow *>(qApp->activeWindow());

        if (!mainWindow->instanceExists(filePath)) {
            mainWindow->openFolderInNewWindow(filePath);
        }

        while (!mainWindow->instanceExists(filePath)) {
            qDebug() << "Waiting for instance of FileManagerMainWindow for" << filePath;
            QApplication::processEvents();
        }

        mainWindow = mainWindow->getInstanceForDirectory(filePath);
    }
}


void DBusInterface::ShowItems(const QStringList &uriList, const QString &startUpId)
{
    qDebug() << "ShowItems" << uriList << startUpId;

    for (QString fileUrlString : uriList) {
        // Convert the URL to a local file path
        QUrl fileUrl(fileUrlString);
        QString filePath = fileUrl.toLocalFile();

        // Get the parent directory
        QFileInfo fileInfo(filePath);
        QString parentDir = fileInfo.absoluteDir().path();
        if (!fileInfo.exists()) {
            QMessageBox::warning(0, 0, tr("File does not exist: %1").arg(filePath));
            continue;
        }

        FileManagerMainWindow* mainWindow = qobject_cast<FileManagerMainWindow*>(qApp->activeWindow());

        if (!mainWindow->instanceExists(parentDir)) {
            mainWindow->openFolderInNewWindow(parentDir);
        }

        // Find the instance of FileManagerMainWindow for parentDir
        // Wait until this succeeds: mainWindow->getInstanceForDirectory(parentDir);
        while (!mainWindow->instanceExists(parentDir)) {
            qDebug() << "Waiting for instance of FileManagerMainWindow for" << parentDir;
            QApplication::processEvents();
        }

        mainWindow = mainWindow->getInstanceForDirectory(parentDir);

        QStringList filePathList = QStringList() << filePath;
        mainWindow->selectItems(filePathList);
    }
}

void DBusInterface::ShowItemProperties(const QStringList &uriList, const QString &startUpId)
{
    qDebug() << "ShowItemProperties" << uriList << startUpId;

    for (QString fileUrlString : uriList) {
        // Convert the URL to a local file path
        QUrl fileUrl(fileUrlString);
        QString filePath = fileUrl.toLocalFile();
        InfoDialog *infoDialog = InfoDialog::getInstance(filePath);
        infoDialog->setAttribute(Qt::WA_DeleteOnClose);
        infoDialog->show();
        // Delete the InfoDialog after it has been closed
        connect(infoDialog, &InfoDialog::destroyed, infoDialog, &QObject::deleteLater);
    }
}

void DBusInterface::SortOrderForUrl(const QString &url, QString &role, QString &order)
{
    QMessageBox::warning(0, 0, "SortOrderForUrl is not implemented yet");
    qDebug() << "SortOrderForUrl" << url << role << order;
}