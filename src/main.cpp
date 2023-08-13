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

#include <QApplication>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QMessageBox>
#include <QStandardPaths>
#include <QSharedMemory>
#include <QDebug>
#include <QTranslator>
#include <QProcess>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QThread>
#include <QString>

#include "FileManagerMainWindow.h"
#include "DBusInterface.h"
#include "ElfSizeCalculator.h"
#include "SqshArchiveReader.h"
#include "AppGlobals.h"
#include "FileOperationManager.h"
#include "VolumeWatcher.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set the application name
    QCoreApplication::setApplicationName("Filer");
    // Set the application version
    QCoreApplication::setApplicationVersion("1.0");

    // Add --version and -v
    QCommandLineParser parser;
    parser.setApplicationDescription("Filer");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    // Allow for arguments to be passed to the application
    // In this case, we don't create a main window, but instead we send messages via D-Bus
    // that cause the already running file manager to open the paths
    QStringList pathsToBeOpened = parser.positionalArguments();
    if (! pathsToBeOpened.isEmpty()) {

        // Create a D-Bus message to call the ShowFolders method
        QDBusMessage message = QDBusMessage::createMethodCall(
                "org.freedesktop.FileManager1",    // Service name
                "/org/freedesktop/FileManager1",  // Object path
                "org.freedesktop.FileManager1",    // Interface name
                "ShowFolders"                        // Method name
        );

        // Set the arguments for the method call
        message << pathsToBeOpened;

        // Send the D-Bus message and get the reply
        QDBusMessage reply = QDBusConnection::sessionBus().call(message);

        if (reply.type() == QDBusMessage::ReplyMessage) {
            qDebug() << "ShowFolders method called successfully";
        } else if (reply.type() == QDBusMessage::ErrorMessage) {
            qDebug() << "Failed to call ShowFolders method: " << reply.errorMessage();
        } else {
            qDebug() << "Failed to call ShowFolders method";
        }
        app.quit();
    }

    // Check whether another instance of this application is already running
    const QString sharedMemoryKey = qApp->applicationName();
    QSharedMemory sharedMemory(sharedMemoryKey);
    qDebug() << "Attempting to attach to shared memory...";
    if (sharedMemory.attach()) {
        qDebug() << "Shared memory segment already exists.";
        QString text = QObject::tr("Another instance of %1 is already running.").arg(qApp->applicationName());
        QMessageBox::critical(0, qApp->applicationName(), text);
        return 1;
    }
    qDebug() << "Trying to create shared memory...";
    if (!sharedMemory.create(1)) {
        qDebug() << "Failed to create shared memory segment.";
        QString text = QObject::tr("Failed to create shared memory segment. Please close any other instances of %1.").arg(qApp->applicationName());
        QMessageBox::critical(0, qApp->applicationName(), text);
        return 1;
    }
    qDebug() << "Shared memory segment created successfully.";
    // Properly detach the shared memory when the application exits
    QObject::connect(&app, &QCoreApplication::aboutToQuit, [&sharedMemory]() {
        sharedMemory.detach();
    });

    // On the $PATH check for the existence of the following commands:
    // - open
    // - launch
    QStringList neededCommands = QStringList() << "open" << "launch";
    foreach (QString neededCommand, neededCommands) {
        if (!QStandardPaths::findExecutable(neededCommand).isEmpty()) {
            // Found
        } else {
            // Not found
            QMessageBox::critical(0, "Filer", QString("The '%1' command is missing. Please install it.").arg(neededCommand));
            return 1;
        }
    }

    if (FileOperationManager::findFileOperationBinary().isEmpty()) {
        return 1;
    }

    // Run "open" without arguments and get its output; check
    // whether it is our version of open and not e.g., xdg-open.
    // Running the "open" command without arguments also populates
    // the launch "database", which is needed for the "launch"
    // command to work and for Filer to be able to draw proper document icons.
    QProcess openProcess;
    openProcess.setProcessChannelMode(QProcess::MergedChannels);
    openProcess.start("open");
    openProcess.waitForFinished();
    QString openOutput = openProcess.readAllStandardOutput();
    if (openOutput.contains("pen <document to be opened>")) {
        // Found
    } else {
        // Not found
        QMessageBox::critical(0, "Filer", QString("The 'open' command is not the one from https://github.com/helloSystem/launch/. Please install it."));
        return 1;
    }

    // Make FileManager1 available on D-Bus
    DBusInterface dbusInterface;

    // Create a FileManagerTreeView instance at ~/Desktop
    FileManagerMainWindow mainWindow;

    // Show the main window
    mainWindow.show();

    // Show volumes on the Desktop
    // FIXME: Replace this by a QProxyModel or something more appropriate

    // Install a QFileSystemWatcher on /media, and whenever a new directory appears, symlink it to ~/Desktop
    // when a directory disappears, remove the symlink
    VolumeWatcher watcher;

    return app.exec();
}