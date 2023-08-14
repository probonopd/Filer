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

#include "LaunchDB.h"
#include <QDir>
#include <QDebug>

LaunchDB::LaunchDB() {
    db = new QMimeDatabase;
}

LaunchDB::~LaunchDB() {
    delete db;
}

QString LaunchDB::applicationForFile(const QFileInfo &fileInfo) const {
    // Check if the file exists
    if (!fileInfo.exists()) {
        return QString(); // Return an empty QString if the file doesn't exist
    }

    // Resolve symlinks and get absolute path
    QString absoluteFilePath = fileInfo.absoluteFilePath();
    // If it is a symlink, then resolve it
    if (fileInfo.isSymLink()) {
        absoluteFilePath = fileInfo.symLinkTarget();
    }

    // Get the MIME type of the file
    QMimeType mimeType = db->mimeTypeForFile(absoluteFilePath);
    // qDebug("");
    // qDebug("'%s' has MIME type '%s'", qPrintable(absoluteFilePath), qPrintable(mimeType.name()));

    // If we have "text/x-pdf, see whether ~/.local/share/launch/MIME/text_x-pdf/ exists (just as an example)
    QString mimeDir = QDir::homePath() + "/.local/share/launch/MIME/" + mimeType.name().replace("/", "_");
    if (QDir(mimeDir).exists()) {
        // If there is a default application for the MIME type, then return it
        QString defaultApplication = mimeDir + "/Default";
        if (QFile(defaultApplication).exists()) {
            defaultApplication = QFileInfo(defaultApplication).absoluteFilePath();
            // If it is a symlink, then resolve it
            if (QFileInfo(defaultApplication).isSymLink()) {
                defaultApplication = QFileInfo(defaultApplication).symLinkTarget();
            }
            if (!QFile(defaultApplication).exists()) {
                return QString();
            }
            return defaultApplication;
        }
        // If there is only one application for the MIME type (not counting the default application), then return it
        QStringList applications = QDir(mimeDir).entryList(QDir::Files);
        // applications.removeAll("Default");
        if (applications.size() == 1) {
            QString application = mimeDir + "/" + applications.at(0);
            if (QFile(application).exists()) {
                application = QFileInfo(application).absoluteFilePath();
                // If it is a symlink, then resolve it
                if (QFileInfo(application).isSymLink()) {
                    application = QFileInfo(application).symLinkTarget();
                }
                if (!QFile(application).exists()) {
                    return QString();
                }
                return application;
            }
        }
    }

    // Return the default application for the file
    return QString(); // Return an empty QString if we don't know the default application
}
