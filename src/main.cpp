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

#include "FileManagerMainWindow.h"
#include "FileManager.h"

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

    // Check whether another instance of this application is already running
    const QString sharedMemoryKey = qApp->applicationName();
    QSharedMemory sharedMemory(sharedMemoryKey);
    if (sharedMemory.attach()) {
        // Show a message box and exit
        // TODO: Instead of doing this, we should check whether the can invoke the running instance with D-Bus
        // and have it open the requested file or director
        QString text = QObject::tr("Another instance of %1 is already running.").arg(qApp->applicationName());
        QMessageBox::critical(0, qApp->applicationName(), text);
        return 0; // Exit the second instance of the application
    }
    if (!sharedMemory.create(1)) {
        // Show a message box and exit
        QString text = QObject::tr("Failed to create shared memory segment. Please close any other instances of %1.").arg(qApp->applicationName());
        QMessageBox::critical(0, qApp->applicationName(), text);
        return 1;
    }

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

    /*
     * FIXME: Why does this cause a crash?
     *
    // Expose services on D-Bus
    FileManager fileManager;
    QDBusConnection::sessionBus().registerService("org.freedesktop.FileManager1");
    QDBusConnection::sessionBus().registerObject("/org/freedesktop/FileManager1", &fileManager);
     */

    // Create a FileManagerTreeView instance at ~/Desktop
    FileManagerMainWindow mainWindow;

    // Show the main window
    mainWindow.show();

    return app.exec();
}