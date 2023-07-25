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

#include "customfileiconprovider.h"
#include "applicationbundle.h"
#include "LaunchDB.h"
#include "CombinedIconCreator.h"
#include "extendedattributes.h"

#include <QDebug>
#include <QFile>
#include <QIcon>
#include <QPainter>


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
        // If the file has the executable bit set and is not a directory,
        // then we always want to show the executable icon
        if (info.isExecutable() && !info.isDir()) {
            return (QIcon::fromTheme("application-x-executable"));
        }
        // Handle .DirIcon (AppDir) and volumelcon.icns (Mac)
        QStringList candidates = {info.absoluteFilePath() + "/.DirIcon", info.absoluteFilePath() + "/volumelcon.icns"};
        for (const QString &candidate: candidates) {
            if (QFileInfo(candidate).exists()) {
                // Read the contents of the file and turn it into an icon
                QFile file(candidate);
                return (QIcon(file.readAll()));
            }
        }
        // If it is a directory, then we always want to show the folder icon
        if (info.isDir()) {
            return (QIcon::fromTheme("folder"));
        }

        // Construct an icon from the default document icon plus the icon of the application that will
        // be used to open the file
        // This is how it should be, so that the user always knows what
        // application will be used to open the file

        CombinedIconCreator iconCreator;

        // Read the extended attributes of the file, see if there is an "open-with" attribute
        // If so, then use that application to open the file

        ExtendedAttributes ea(info.absoluteFilePath());
        QByteArray openWith = ea.read("open-with");

        if (!openWith.isEmpty()) {
            qDebug() << "openWith:" << openWith << "for" << info.absoluteFilePath();
            ApplicationBundle bundle(openWith);
            if (bundle.isValid()) {
                QIcon applicationIcon = QIcon(bundle.icon()).pixmap(16, 16);
                QIcon combinedIcon = iconCreator.createCombinedIcon( applicationIcon);
                return (combinedIcon);
            }
        }

        // Find out which application will be used to open the file by default from the launch database
        LaunchDB ldb;
        QString application = ldb.applicationForFile(info);
        qDebug() << "application:" << application << "for" << info.absoluteFilePath();
        // If we did not find an application, then we use the "?" icon
        if (application.isEmpty()) {
            return (QIcon::fromTheme("unknown"));
        } else {
            // Get the icon of the application
            ApplicationBundle bundle(application);
            if (bundle.isValid()) {

                /*
                // Overlay the icon of the application over the document icon
                QPixmap document_icon = QIcon::fromTheme("document").pixmap(32, 32);
                QPixmap application_icon = QIcon(bundle.icon()).pixmap(16, 16);
                QPixmap combined_icon(32, 32);
                combined_icon.fill(Qt::transparent);
                QPainter painter(&combined_icon);
                painter.drawPixmap(0, 0, document_icon);
                painter.drawPixmap(8, 8, application_icon);
                painter.end();
                return (QIcon(combined_icon));
                 */

                QIcon applicationIcon = QIcon(bundle.icon()).pixmap(16, 16);
                QIcon combinedIcon = iconCreator.createCombinedIcon( applicationIcon);
                return (combinedIcon);

            }
        }
    }
/*
        // As a fallback, try to load the icon from the icon theme.
        // TODO: We may want to remove this once using the launch database works satisfactorily
        // Load icons for mime types from the icon theme in a deterministic way. Note that we need to make many
        // hardcoded changes; why is this? FIXME: Can we get the proper icon names from somewhere?
        QString mimedb_icon_name = db.mimeTypeForFile(info).iconName();
        // qDebug() << "mimedb_icon_name: " << mimedb_icon_name << " for " << info.absoluteFilePath();
        QStringList icon_name_candidates = {mimedb_icon_name};
        // Note that we get, e.g., "text-x-python3" but we only have "text-x-python.png" in the icon theme
        // So we need to strip the "3" from the icon name - just as an example
        QString mimedb_icon_name_without_version = mimedb_icon_name;
        mimedb_icon_name_without_version.remove(QRegExp("[0-9*?]$"));
        icon_name_candidates.append(mimedb_icon_name_without_version);
        // Also note that we get "image-svg+xml" but we only have "image-x-svg+xml.png" in the icon theme
        // So we need to add the "x-" to the icon name after the first "-" - just as an example (not limited to svg)
        if (mimedb_icon_name.contains("-") && !mimedb_icon_name.contains("-x-")) {
            QString mimedb_icon_name_with_x = mimedb_icon_name;
            mimedb_icon_name_with_x.insert(mimedb_icon_name.indexOf("-") + 1, "x-");
            icon_name_candidates.append(mimedb_icon_name_with_x);
        }
        // Also note that we get "audio-mp4" but we only have "audio-mpeg.png" in the icon theme
        // So we need to replace the "mp[0-9]" with "mpeg" in the icon name
        if (mimedb_icon_name.contains("mp")) {
            QString mimedb_icon_name_with_mpeg = mimedb_icon_name;
            mimedb_icon_name_with_mpeg.replace(QRegExp("mp[0-9]"), "mpeg");
            icon_name_candidates.append(mimedb_icon_name_with_mpeg);
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
                            QString("/") + mimedb_icon_name +
                            QString(".*");
        qDebug() << "No icon found for mime type" << mimedb_icon_name << "in path" << icon_path;
    }
*/
    // As a last resort, return "?" icon for everything else
    return (QIcon::fromTheme("unknown"));
}