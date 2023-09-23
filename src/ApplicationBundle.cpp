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
#include <QPainter>

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
                QStringList icons = resourcesDir.entryList();
                // qDebug() << icons;
                if (icons.size() > 0) {
                    m_icon = resourcesDir.filePath(icons.at(0));
                }
            }
            m_executable = dir.filePath(fileInfo.completeBaseName());
        } else if (QFileInfo(dir.filePath("AppRun")).isExecutable()) {
            m_type = Type::AppDir;
            // qDebug() << path << "is an AppDir";
            m_name = QFileInfo(dir.path()).completeBaseName();
            // qDebug() << "Name:" << m_name;
            // Check if the AppDir contains a .DirIcon file
            if (dir.exists(".DirIcon")) {
                m_icon = dir.filePath(".DirIcon");
            }
            m_executable = dir.filePath("AppRun");
        }
    }

    // Check if the path is an AppImage
    // TODO: Use MIME type instead of file extension; measure performance impact
    if (fileInfo.fileName().toLower().endsWith(".appimage")) {
        m_type = Type::AppImage;
        m_name = fileInfo.completeBaseName();
        m_executable = fileInfo.fileName();
    }

    // Check if the path is a desktop file
    // TODO: Use MIME type instead of file extension; measure performance impact
    if (fileInfo.fileName().toLower().endsWith(".desktop")) {
        m_type = Type::DesktopFile;
        // qDebug() << path << "is a desktop file";
        m_name = fileInfo.completeBaseName();
        // qDebug() << "Name:" << m_name;
        m_icon = DesktopFile::getIcon(fileInfo.filePath());
        // The next line is overly simplistic, but works for most cases
        m_executable = DesktopFile::getValue(fileInfo.filePath(), "Exec").split("%").at(0);
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
        if (icon.isNull()) {
            // If the icon is not found in the theme, return the default icon
            return QIcon::fromTheme("application-x-executable");
        }
        return icon;
    } else if (m_type == Type::AppImage) {
        // Determine the ELF offset
        qint64 offset = ElfSizeCalculator::calculateElfSize(m_path);
        // qDebug() << "offset:" << offset << "for file" << m_path;
        // Get the data of the .DirIcon file from the squashfs
        SqshArchiveReader *reader = new SqshArchiveReader(offset);
        qDebug() << "Extracting AppImage icon for file" << m_path;
        QByteArray fileData = reader->readFileFromArchive(m_path, ".DirIcon");
        // qDebug() << "Finished extracting AppImage icon data for file" << m_path;
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
            qDebug() << "Icon image is not null for file" << m_path;
            // return quadraticIcon(QPixmap::fromImage(image));
            return QIcon(QPixmap::fromImage(image));
        }
    } else {
        // Get the icon from the icon file if it exists
        if (m_icon.isEmpty()) {
            // Get the default icon if the icon file does not exist
            return QIcon::fromTheme("application-x-executable");
        }
        QFile file(m_icon);
        if (file.exists()) {
            // If the icon is not quadratic, extend it to a quadratic shape and align it in the center bottom
            return quadraticIcon(QIcon(m_icon));
        } else {
            // Get the default icon if the icon file does not exist
            return QIcon::fromTheme("application-x-executable");
        }
    }
}

QIcon ApplicationBundle::quadraticIcon(QIcon icon) const {
    if (QPixmap(m_icon).width() != QPixmap(m_icon).height()) {
        QPixmap pixmap(m_icon);
        QPixmap squaredPixmap = QPixmap(pixmap.width(), pixmap.width());
        squaredPixmap.fill(Qt::transparent);
        QPainter painter(&squaredPixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.drawPixmap((pixmap.width() - pixmap.height()) / 2, 0, pixmap);
        return QIcon(squaredPixmap);
    } else {
        return QIcon(m_icon);
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

bool ApplicationBundle::launch(QStringList arguments) const
{
    qDebug() << "Launching" << m_path << "with arguments" << arguments;
    QProcess *process = new QProcess();
    process->setProgram("launch");
    process->setArguments(QStringList() << m_executable << arguments);
    process->start();
    if(!process->waitForStarted()) {
        qDebug() << "Error starting process";
        return false;
    }
    return true;
}
