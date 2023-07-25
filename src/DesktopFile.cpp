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

#include "DesktopFile.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QTextStream>
#include <QDebug>
#include <QIcon>

DesktopFile::DesktopFile(const QString &filename) : m_filename(filename) { }

QString DesktopFile::getIcon() const
{
    QString icon;
    QFile file(m_filename);
    qDebug() << "Searching for icon for" << m_filename;
    // Search for a line starting with "Icon=" in the file
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            qDebug() << line;
            if (line.startsWith("Icon")) {
                // Split the line at "=" and take the second part, trim it
                icon = line.split("=").at(1).trimmed();
                break;
            }
        }
        file.close();
    }

    // Print error message if icon is not found
    if (icon.isEmpty()) {
        qDebug() << "Icon= not found in" << m_filename;
    } else {
        qDebug() << "Searching typical xdg locations for" << icon;
    }

    if (!icon.isEmpty()) {
        // Search for the icon in directories that may contain icons
        QStringList iconPaths;
        QString currentThemeName = QIcon::themeName();
        qDebug() << "currentThemeName: " << currentThemeName;
        iconPaths << "/usr/share/pixmaps"
                  << QString("/usr/share/icons") + QDir::separator() + currentThemeName
                  << QString("/usr/share/icons") + QDir::separator() + "hicolor" // Fallback icon theme
                  << "/usr/local/share/pixmaps"
                  << QString("/usr/local/share/icons") + QDir::separator() + currentThemeName
                  << QString("/usr/local/share/icons") + QDir::separator() + "hicolor" // Fallback icon theme
                  << QDir::homePath() + QDir::separator() + ".icons" // User's icon directory
                  << QDir::homePath() + QDir::separator() + ".local/share/pixmaps" // User's pixmap directory
                  << QDir::homePath() + QDir::separator() + ".local/share/icons"; // User's icon theme directory

        // List of allowed icon suffixes
        QStringList allowedSuffixes;
        allowedSuffixes << ".png"
                        << ".svg"
                        << ".xpm"
                        << ".ico"
                        << ".icns";

        // List of additional sizes based on XDG icon theme specification
        QStringList iconSizes;
        iconSizes << ""
                  << "32x32"
                  << "48x48"
                  << "64x64"
                  << "512x512"
                  << "256x256"
                  << "128x128"
                  << "scalable"
                  << "24x24"
                  << "22x22"
                  << "16x16";

        // Check each directory in iconPaths and for each size
        for (const QString &path : iconPaths) {
            for (const QString &size : iconSizes) {
                for (const QString &suffix : allowedSuffixes) {
                    QString iconFile = path + "/" + size + "/apps/" + icon + suffix;
                    if (QFile::exists(iconFile)) {
                        icon = iconFile;
                        qDebug() << "Found icon" << icon;
                        return icon;
                    }
                }
            }
        }
    }

    return icon;
}
