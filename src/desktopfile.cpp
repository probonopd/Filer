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

#include "desktopfile.h"
#include <QFile>
#include <QStringList>
#include <QTextStream>

DesktopFile::DesktopFile(const QString &filename) : m_filename(filename) { }

QString DesktopFile::getIcon() const
{
    QString icon;
    QFile file(m_filename);
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        while (!stream.atEnd()) {
            QString line = stream.readLine();
            if (line.startsWith("Icon=")) {
                icon = line.mid(5); // get the value after "Icon="
                break;
            }
        }
    }

    if (!icon.isEmpty()) {
        // Search for the icon in directories that may contain icons
        QStringList iconPaths;
        iconPaths << "/usr/share/pixmaps"
                  << "/usr/share/icons"
                  << "/usr/local/share/pixmaps"
                  << "/usr/local/share/icons";

        // List of allowed icon suffixes
        QStringList allowedSuffixes;
        allowedSuffixes << ".png"
                        << ".svg"
                        << ".xpm"
                        << ".ico"
                        << ".icns";

        for (const QString &path : iconPaths) {
            for (const QString &suffix : allowedSuffixes) {
                QString iconFile = path + "/" + icon + suffix;
                if (QFile::exists(iconFile)) {
                    icon = iconFile;
                    break;
                }
            }
        }
    }

    return icon;
}
