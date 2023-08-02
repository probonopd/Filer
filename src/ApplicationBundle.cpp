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

        DesktopFile df(fileInfo.filePath());
        m_icon = df.getIcon();
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
    qDebug() << "m_icon:" << m_icon;
    if (m_type == Type::DesktopFile) {
        // Get the icon from the theme if it is a desktop file
        QIcon icon = QIcon::fromTheme(m_icon);
        return icon;
    } else {
        // Get the icon from the icon file if it exists
        QFile file(m_icon);
        if (file.exists()) {
            QIcon icon(m_icon);
            return icon;
        } else {
            // Get the default icon if the icon file does not exist
            QIcon icon = QIcon::fromTheme("application-x-executable");
            return icon;
        }
    }
}

QString ApplicationBundle::iconName() const
{
    qDebug() << "m_icon:" << m_icon;
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