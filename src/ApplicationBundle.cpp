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

#include "ApplicationBundle.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QSettings>
#include <QFile>
#include <QIcon>
#include <QStandardPaths>
#include <QDebug>

#include "ElfSizeCalculator.h"
#include "SqshArchiveReader.h"

#include <DesktopFile.h>

ApplicationBundle::ApplicationBundle(const QString& path)
        : m_path(path),
          m_isValid(false),
          m_type(Type::Unknown),
          m_name(),
          m_icon(),
          m_executable(),
          m_arguments()
{
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        return;
    }

    // Check if the path is an application bundle or AppDir
    if (fileInfo.isDir()) {
        QDir dir(path);
        // qDebug() << "Checking if" << path << "is an application bundle or AppDir";
        if (dir.exists("Resources")
            && QFileInfo(dir.filePath(fileInfo.completeBaseName())).isExecutable()) {
            m_type = Type::AppBundle;
            // qDebug() << path << "is an application bundle";
            m_name = QFileInfo(dir.path()).completeBaseName();
            // qDebug() << "Name:" << m_name;

            // Check if the Resources directory contains an icon file with the same name as the dir
            QDir resourcesDir(dir.filePath("Resources"));
            // qDebug() << "resourcesDir: " << resourcesDir;
            QStringList filters;
            filters << m_name + ".png" << m_name + ".jpg" << m_name + ".svg" << m_name + ".svgz"
                    << m_name + ".ico" << m_name + ".icns";
            resourcesDir.setNameFilters(filters);
            if (resourcesDir.exists()) {
                m_icon = resourcesDir.filePath(resourcesDir.entryList(filters).first());
            }
        } else if (QFileInfo(dir.filePath("AppRun")).isExecutable()) {
            m_type = Type::AppDir;
            // qDebug() << path << "is an AppDir";
            m_name = QFileInfo(dir.path()).completeBaseName();
            // qDebug() << "Name:" << m_name;

            // Check if the AppDir contains a .DirIcon file
            if (dir.exists(".DirIcon")) {
                m_icon = dir.filePath(".DirIcon");
            }
        }
    }

    // Check if the path is an AppImage
    if (fileInfo.fileName().endsWith(".AppImage")) {
        m_type = Type::AppImage;
        m_name = fileInfo.completeBaseName();
    }

    // Check if the path is a desktop file
    if (fileInfo.fileName().endsWith(".desktop")) {
        m_type = Type::DesktopFile;
        // qDebug() << path << "is a desktop file";
        m_name = fileInfo.completeBaseName();
        // qDebug() << "Name:" << m_name;

        m_icon = DesktopFile::getIcon(fileInfo.filePath());
    }

    if (m_type != Type::Unknown) {
        m_isValid = true;
    } else {
        m_isValid = false;
    }
}

QString ApplicationBundle::path() const
{
    return m_path;
}

bool ApplicationBundle::isValid() const
{
    return m_isValid;
}

ApplicationBundle::Type ApplicationBundle::type() const
{
    return m_type;
}

QIcon ApplicationBundle::icon() const
{
    // qDebug() << "m_icon:" << m_icon;
    if (m_type == Type::DesktopFile) {
        // Get the icon from the theme if it is a desktop file
        QIcon icon = QIcon::fromTheme(m_icon);
        return icon;
    } else if (m_type == Type::AppImage) {
        // Determine the ELF offset
        qint64 offset = ElfSizeCalculator::calculateElfSize(m_path);
        qDebug() << "offset:" << offset << "for file" << m_path;
        // Get the data of the .DirIcon file from the squashfs
        SqshArchiveReader *reader = new SqshArchiveReader(offset);
        qDebug() << "Extracting AppImage icon for file" << m_path;
        QByteArray fileData = reader->readFileFromArchive(m_path, ".DirIcon");
        qDebug() << "Finished extracting AppImage icon data for file" << m_path;
        delete reader;
        // If fileData is empty, return default icon
        if (fileData.isEmpty()) {
            return QIcon::fromTheme("application-x-executable");
            qDebug() << "Icon fileData is empty for file" << m_path;
        }
        // Turn the data of the .DirIcon into a QIcon
        QImage image = QImage::fromData(fileData);
        if (image.isNull()) {
            return QIcon::fromTheme("application-x-executable");
            qDebug() << "Icon image is null for file" << m_path;
        } else {
            return QIcon(QPixmap::fromImage(image));
        }
    } else {
        // Get the icon from the icon file if it exists
        QFile file(m_icon);
        if (file.exists()) {
            return QIcon(m_icon);
        } else {
            // Get the default icon if the icon file does not exist
            return QIcon::fromTheme("application-x-executable");
        }
    }
}

QString ApplicationBundle::iconName() const
{
    // qDebug() << "m_icon:" << m_icon;
    return m_icon;
}


QString ApplicationBundle::name() const
{
    return m_name;
}

QString ApplicationBundle::executable() const
{
    return m_executable;
}

QStringList ApplicationBundle::arguments() const
{
    return m_arguments;
}

bool ApplicationBundle::isCommandLineTool() const
{
    if (m_type == Type::DesktopFile) {
        return DesktopFile::isCommandLineTool(m_path);
    } else if (m_type == Type::AppImage) {
        qint64 offset = ElfSizeCalculator::calculateElfSize(m_path);
        qDebug() << "Offset:" << offset << "for file" << m_path;

        SqshArchiveReader *reader = new SqshArchiveReader(offset);
        qDebug() << "Extracting desktop files from" << m_path;

        QStringList desktopFileCandidates = reader->readSqshArchive(m_path);
        qDebug("desktopFileCandidates: %s", qUtf8Printable(desktopFileCandidates.join(", ")));

        for (const QString desktopFileCandidate : desktopFileCandidates) {
            // If ends with .desktop, check if it is a command line tool
            if (desktopFileCandidate.endsWith(".desktop")) {
                QString desktopFileContents = reader->readFileFromArchive(m_path, desktopFileCandidate);
                qDebug() << "desktopFileContents:" << desktopFileContents;
                QStringList lines = desktopFileContents.split("\n");
                for (QString line : lines) {
                    if (line.startsWith("Terminal=")) {
                        // Split the line at "=" and take the second part, trim it
                        QString value = line.split("=").at(1).trimmed();
                        if (value == "true") {
                            delete reader;
                            return true;
                        } else {
                            delete reader;
                            return false;
                        }
                    }
                }
            }
        }
        delete reader;
        return false;
    }
    return false;
}

QString ApplicationBundle::typeName() const {
    switch (m_type) {
    case Type::Unknown:
        return tr("Application");
    case Type::AppBundle:
        return tr("Application") + " (.app bundle)";
    case Type::AppDir:
        return tr("Application") + " (AppDir)";
    case Type::AppImage:
        return tr("Application") + " (AppImage)";
    case Type::DesktopFile:
        return tr("Application") + " (.desktop file)";
    }
    return "Unknown";
}