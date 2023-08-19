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

#include "CustomFileIconProvider.h"
#include "CustomFileSystemModel.h"
#include "ApplicationBundle.h"

#include "CombinedIconCreator.h"
#include "ExtendedAttributes.h"

#include <QDebug>
#include <QProcess>
#include <QFile>
#include <QIcon>
#include <QPainter>
#include <QApplication>
#include <QStorageInfo>
#include <QThread>
#include <QImageReader>
#include "AppGlobals.h"

CustomFileIconProvider::CustomFileIconProvider()
        : iconCreator(new CombinedIconCreator) // "Initialize the pointer in the constructor"
{

    currentThemeName = QIcon::themeName();
    // qDebug() << "currentThemeName: " << currentThemeName;

    CombinedIconCreator iconCreator;
    m_model = nullptr;
}

CustomFileIconProvider::~CustomFileIconProvider()
{
    delete iconCreator; // Release the memory in the destructor
}

/**
 * @brief CustomFileIconProvider::icon  Returns the icon for the given file info
 * @param info  The file info
 * @return  The icon
 */
QIcon CustomFileIconProvider::icon(const QFileInfo &info) const
{
    qDebug() << "CustomFileIconProvider::icon: " << info.absoluteFilePath();

    // Check if the item is an application bundle and return the icon
    ApplicationBundle *bundle = new ApplicationBundle(info.absoluteFilePath());
    // Schedule bundle for deletion
    bundle->moveToThread(QApplication::instance()->thread());
    QMetaObject::invokeMethod(bundle, "deleteLater", Qt::QueuedConnection);

    if (bundle->isValid()) {
        // qDebug() << "Bundle is valid: " << info.absoluteFilePath();
        return (bundle->icon());
    }

    // Check if the path is a mount point using QStorageInfo
    QStringList mountPoints;
    for (const QStorageInfo &storage : QStorageInfo::mountedVolumes()) {
        mountPoints << storage.rootPath();
    }
    QString absoluteFilePathWithSymLinksResolved = info.absoluteFilePath();
    if (info.isSymLink()) {
        absoluteFilePathWithSymLinksResolved = info.symLinkTarget();
    }

    // How many directories deep is AppGlobals::mediaPath?
    int mediaPathDepth = AppGlobals::mediaPath.count("/");

    // How many directories deep is the current file?
    int fileDepth = absoluteFilePathWithSymLinksResolved.count("/");

    // Is the current file in a subdirectory of AppGlobals::mediaPath but not in a sub-subdirectory?
    bool isMediaPath = (absoluteFilePathWithSymLinksResolved.startsWith(AppGlobals::mediaPath) && (fileDepth == mediaPathDepth + 1));

    // If it is a directory and the symlink target is a mount point, then we want to show the drive icon
    if (info.isDir() && isMediaPath) {
        // Using Qt, get the device node of the mount point
        // and then use the device node to get the icon
        // qDebug() << "Mount point: " << info.absoluteFilePath();
        QStorageInfo storageInfo(info.absoluteFilePath());
        QString deviceNode = storageInfo.device();
        qDebug() << "Device node: " << deviceNode;

        // Set the icon depending on the file system type; unlike device nodes,
        // this also works for mounted disk images
        QString fileSystemType = storageInfo.fileSystemType();
        qDebug() << "File system type: " << fileSystemType;
        if (fileSystemType == "iso9660" | fileSystemType == "udf" | fileSystemType == "cd9660") {
            return (QIcon::fromTheme("media-optical"));
        }

        // Set the icon depending on the device node
        if (deviceNode.startsWith("/dev/da")){
            return (QIcon::fromTheme("drive-removable-media"));
        } else if (deviceNode.startsWith("/dev/sr") || deviceNode.startsWith("/dev/cd")) {
            return (QIcon::fromTheme("media-optical"));
        } else {
            return (QIcon::fromTheme("drive-harddisk"));
        }
    }

    if (info.isDir() && mountPoints.contains(absoluteFilePathWithSymLinksResolved)) {
        // Using Qt, get the device node of the mount point
        // and then use the device node to get the icon
        // qDebug() << "Mount point: " << info.absoluteFilePath();
        QStorageInfo storageInfo(info.absoluteFilePath());
        QString deviceNode = storageInfo.device();
        qDebug() << "Device node: " << deviceNode;
        if (deviceNode.startsWith("/dev/da")){
            return (QIcon::fromTheme("drive-removable-media"));
        } else if (deviceNode.startsWith("/dev/sr") || deviceNode.startsWith("/dev/cd")) {
            return (QIcon::fromTheme("media-optical"));
        } else {
            return (QIcon::fromTheme("drive-harddisk"));
        }
    }

    // If it is not a bundle but a directory, then we want to show the folder icon
    if (info.isDir()) {
        // If it is lacking permissions, then we want to show the locked folder icon; TODO: Use emblem instead?
        if (!QFileInfo(info.absoluteFilePath()).isReadable() || !QFileInfo(info.absoluteFilePath()).isExecutable()) {
            // Try to get folder-locked icon from the current theme,
            // fall back to other icons if it is not available
            if (QIcon::hasThemeIcon("folder-locked")) {
                return (QIcon::fromTheme("folder-locked"));
            } else if (QIcon::hasThemeIcon("lock")) {
                return (QIcon::fromTheme("lock"));
            } else {
                return (QIcon::fromTheme("cancel"));
            }
        } else {
            return (QIcon::fromTheme("folder"));
        }
    }

    // If we have no read permissions, show the lock icon; TODO: Use emblem instead?
    if (!QFileInfo(info.absoluteFilePath()).isReadable()) {
        // Try to get lock icon from the current theme,
        // fall back to other icons if it is not available
        if (QIcon::hasThemeIcon("lock")) {
            return (QIcon::fromTheme("lock"));
        } else {
            return (QIcon::fromTheme("cancel"));
        }
    }

    // If it is an .exe file, then we want to show the ICO from the .exe file
    if (info.suffix().compare("exe", Qt::CaseInsensitive) == 0)
    {
        qDebug() << "File extension is .exe: " << info.absoluteFilePath();

        // Call icoextract executable to extract the icon
        QProcess icoExtractProcess;
        QStringList arguments;
        arguments << info.absoluteFilePath() << "/tmp/icoextract.ico";
        icoExtractProcess.start("icoextract", arguments);
        if (!icoExtractProcess.waitForStarted() || !icoExtractProcess.waitForFinished())
        {
            qDebug() << "Failed to run icoextract.";
            return QIcon::fromTheme("application-x-ms-dos-executable");
        }

        // Read the extracted icon
        QFile iconFile("/tmp/icoextract.ico");
        if (!iconFile.open(QIODevice::ReadOnly))
        {
            qDebug() << "Failed to open icon file.";
            return QIcon::fromTheme("application-x-ms-dos-executable");
        }
        QByteArray iconData = iconFile.readAll();
        iconFile.close();
        qDebug() << "Icon data size: " << iconData.size();

        // Load the icon data into a QImage
        QImage iconImage;
        if (iconImage.loadFromData(reinterpret_cast<const uchar*>(iconData.constData()), iconData.size(), "ico"))
        {
            // Create a QIcon from the QImage
            QIcon extractedIcon;
            extractedIcon.addPixmap(QPixmap::fromImage(iconImage));
            return extractedIcon;
        }
        else
        {
            qDebug() << "Failed to load icon image.";
            return QIcon::fromTheme("application-x-ms-dos-executable");
        }
    }

    // If the file has the executable bit set and is not a directory,
    // then we always want to show the executable icon
    if (info.isExecutable() && !info.isDir()) {
        // Try to load the application icon from the path ./Resources/application.png relative to the application executable path
        // If the icon cannot be loaded, use the default application icon from the icon theme
        QString applicationPath = QApplication::applicationDirPath();
        QIcon applicationIcon;
        QString applicationIconPath = applicationPath + "/Resources/application.png";
        if (QFile::exists(applicationIconPath)) {
            applicationIcon = QIcon(applicationIconPath);
        } else {
            applicationIcon = QIcon::fromTheme("application-x-executable");
        }
        return (applicationIcon);
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

    // Construct an icon from the default document icon plus the icon of the application that will
    // be used to open the file

    QString filePath = info.absoluteFilePath();

    /*
     * It seems like we may be running into some timing issue, where the file is not yet in the model
     * at the point in time when the icon is requested (but shortly after).
     * For this reason, we are not using the index for now, but the file path directly.
     * This is not ideal, because we are not using the index, but it seems to work for now.
     * FIXME: Find out why the index is not valid at this point in time.
    QModelIndex index = m_model->index(filePath);
    qDebug() << "filePath:" << filePath << "index:" << index;
    if(!index.isValid()) {
        // Does this mean that at the point in time when the icon was requested, the file was not yet in the model?
        qDebug() << "SHOULD NEVER END UP HERE";
        // Return warning icon from theme
        return (QIcon::fromTheme("dialog-warning"));
    }
    */

    qDebug() << "filePath alive:" << filePath;
    if (m_model != nullptr) {
        // Retrieve the "open-with" attribute from the stored attributes in the model.
        QString openWith = QString(m_model->openWith(
                filePath)); // NOTE: We would like to do this with the index, but we don't have a valid index at this point for unknown reasons
        qDebug() << "openWith: " << openWith;
        if (!openWith.isEmpty()) {
            // qDebug() << "-> openWith:" << openWith << "for" << info.absoluteFilePath();
            ApplicationBundle bundle(openWith);
            if (bundle.isValid()) {
                // qDebug("Info: %s is a valid application bundle", qPrintable(openWith));
                QIcon applicationIcon = QIcon(bundle.icon()).pixmap(16, 16);
                if (applicationIcon.isNull()) {
                    qDebug("Warning: %s does not have an icon", qPrintable(openWith));
                    applicationIcon = QIcon::fromTheme("unknown");
                }
                // return(applicationIcon);
                QIcon combinedIcon = iconCreator->createCombinedIcon(applicationIcon);
                return (combinedIcon);
            } else {
                // qDebug("Info: %s is not a valid application bundle", qPrintable(openWith));
            }
        }
        qDebug() << "openWith is empty for " << info.absoluteFilePath();
    } else {
        qDebug() << "m_model is null; may need to set it";
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
    qDebug() << "No icon found for file" << info.absoluteFilePath();
    return (QIcon::fromTheme("unknown"));
}

void CustomFileIconProvider::setModel(CustomFileSystemModel* model)
{
    // Since we need to access the CustomFileSystemModel from the icon provider so that we can call openWith() on it,
    // we need to make it accessible to the icon provider
    m_model = model;
}