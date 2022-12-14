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

#include <QApplication>
#include <QCoreApplication>
#include <QCommandLineParser>

#include "mainwindow.h"
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