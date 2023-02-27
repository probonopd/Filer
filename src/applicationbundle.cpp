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

#include "applicationbundle.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QSettings>
#include <QFile>
#include <QIcon>
#include <QStandardPaths>
#include <QDebug>

#include <desktopfile.h>

ApplicationBundle::ApplicationBundle(const QString &path)
    : m_path(path),
      m_isValid(false),
      m_isApp(false),
      m_isAppDir(false),
      m_isAppImage(false),
      m_name(),
      m_icon(),
      m_executable(),
      m_arguments()
{
    // Check if the path is a valid file or directory
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        return;
    }

    // Check if the path is an application bundle or AppDir
    if (fileInfo.isDir()) {
        QDir dir(path);
        // qDebug() << "Checking if" << path << "is an application bundle or AppDir";
        if (dir.exists("Resources") && QFileInfo(dir.filePath(fileInfo.completeBaseName())).isExecutable()) {
            m_isApp = true;
            // qDebug() << path << "is an application bundle";
            m_name = QFileInfo(dir.path()).completeBaseName();
            // qDebug() << "Name:" << m_name;

            // Check if the Resources directory contains an icon file with the same name as the dir
            QDir resourcesDir(dir.filePath("Resources"));
            // qDebug() << "resourcesDir: " << resourcesDir;
            QStringList filters;
            filters << m_name + ".png"
                    << m_name + ".jpg"
                    << m_name + ".svg"
                    << m_name + ".svgz"
                    << m_name + ".ico"
                    << m_name + ".icns";
            resourcesDir.setNameFilters(filters);
            if (resourcesDir.exists()) {
                m_icon = resourcesDir.filePath(resourcesDir.entryList(filters).first());
            }
        } else if (QFileInfo(dir.filePath("AppRun")).isExecutable()) {
            m_isAppDir = true;
            // qDebug() << path << "is an AppDir";
            m_name = QFileInfo(dir.path()).completeBaseName();
            // qDebug() << "Name:" << m_name;

            // Check if the AppDir contains a .DirIcon file
            if (dir.exists(".DirIcon")) {
                m_icon = dir.filePath(".DirIcon");
            }
        } else {
            m_isApp = false;
            m_isAppDir = false;
        }
    } 

    // Check if the path is an AppImage
    if (fileInfo.fileName().endsWith(".AppImage")) {
        m_isAppImage = true;
        m_name = fileInfo.completeBaseName();
    } else {
        m_isAppImage = false;
    }

    // Check if the path is a desktop file
    if (fileInfo.fileName().endsWith(".desktop")) {
        m_isDesktopFile = true;
        // qDebug() << path << "is a desktop file";
        m_name = fileInfo.completeBaseName();
        // qDebug() << "Name:" << m_name;

        DesktopFile df(fileInfo.fileName());
        m_icon = df.getIcon();
    } else {
        m_isDesktopFile = false;
    }

    // Check if the path is a valid bundle
    if (m_isApp || m_isAppDir || m_isAppImage || m_isDesktopFile) {
        // qDebug() << path << "m_isApp:" << m_isApp << "m_isAppDir:" << m_isAppDir << "m_isAppImage:" << m_isAppImage << "m_isDesktopFile:" << m_isDesktopFile;

        // The path is a recognized application bundle
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

bool ApplicationBundle::isAppBundle() const
{
    return m_isApp;
}

bool ApplicationBundle::isAppDir() const
{
    return m_isAppDir;
}

bool ApplicationBundle::isAppImage() const
{
    return m_isAppImage;
}

QIcon ApplicationBundle::icon() const
{
    return QIcon(m_icon);
}
