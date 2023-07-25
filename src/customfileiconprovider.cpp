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

#include "customfileiconprovider.h"
#include "applicationbundle.h"

#include <QDebug>

CustomFileIconProvider::CustomFileIconProvider()
{
    QMimeDatabase db;

    currentThemeName = QIcon::themeName();
    qDebug() << "currentThemeName: " << currentThemeName;
}

/**
 * @brief CustomFileIconProvider::icon  Returns the icon for the given file info
 * @param info  The file info
 * @return  The icon
 */
QIcon CustomFileIconProvider::icon(const QFileInfo &info) const
{
    // Check if the item is an application bundle and return the icon
    ApplicationBundle bundle(info.absoluteFilePath());
    if (bundle.isValid()) {
        // qDebug() << "Bundle is valid: " << info.absoluteFilePath();
        return (QIcon(bundle.icon()));
    } else {
        // Load icons for mime types in a deterministic way
        QString icon_name = db.mimeTypeForFile(info).iconName();
        QStringList icon_name_candidates = {icon_name};
        // Note that we get, e.g., "text-x-python3" but we only have "text-x-python.png" in the icon theme
        // So we need to strip the "3" from the icon name - just as an example
        icon_name_candidates.append(icon_name.remove(QRegExp("[0-9*?]$")));
        // Also note that we get "image-svg+xml" but we only have "image-x-svg+xml.png" in the icon theme
        // So we need to add the "x-" to the icon name after the first "-" - just as an example (not limited to svg)
        if (icon_name.contains("-")) {
            icon_name_candidates.append(icon_name.replace("-", "-x-"));
        }
        for (int i = 0; i < icon_name_candidates.size(); i++) {
            QList<int> sizes = {32};
            for (int j = 0; j < sizes.size(); j++) {
                // icns, png, svg, xpm
                QStringList extensions = {"icns", "png", "svg", "xpm"};
                for (int k = 0; k < extensions.size(); k++) {
                    QString icon_path = QString("/usr/local/share/icons/" + currentThemeName + "/mimes/") +
                            QString::number(sizes[j]) + QString("/") + icon_name_candidates[i] +
                            QString(".") + extensions[k];
                    // Check if the icon exists
                    QFileInfo icon_info(icon_path);
                    if (icon_info.exists()) {
                        QIcon icon(icon_path);
                        return (icon);
                    }
                }
            }
        }
        QString icon_path = QString("/usr/local/share/icons/" + currentThemeName + "/mimes/32/") +
                            QString("/") + icon_name +
                            QString(".*");
        qDebug() << "No icon found for mime type" << icon_name << "in path" << icon_path;
    }

    // TODO:
    // Construct an icon from the default document icon plus the icon of the application that will
    // be used to open the file This is how it should be, so that the user always knows what
    // application will be used to open the file QString document_icon_path =
    // QString("/usr/local/share/icons/elementary-xfce/mimes/32/gtk-file.png"); Now we would need
    // information from the launch database to determine the application that will be used to open
    // the file

    // If the file is a directory, check for the presence of a .DirIcon file
    // If it exists, return the icon specified in the file

    // candidate = info.absoluteFilePath() + "/.DirIcon"
    QString candidate = info.absoluteFilePath() + "/.DirIcon";
    if (QFileInfo(candidate).exists()) {
        // Read the contents of the file and turn it into an icon
        QFile file(candidate);
    }

    // As a last resort, return the default icon provided by the superclass
    // For whatever reason, this only works for directories and shows a generic file icon for
    // everything else
    return (QFileIconProvider::icon(info));
}