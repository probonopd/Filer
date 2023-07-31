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

#ifndef APPLICATIONBUNDLE_H
#define APPLICATIONBUNDLE_H

#include <QIcon>
#include <QString>
#include <QStringList>

// Represents an application bundle, AppDir, AppImage, or .desktop file
class ApplicationBundle
{
public:
    // Constructs a new ApplicationBundle object with the given path
    ApplicationBundle(const QString &path);

    // Returns the path of the bundle
    QString path() const;

    // Returns true if the bundle is valid, i.e. it is an application bundle, AppDir, AppImage, or
    // .desktop file
    bool isValid() const;

    // Returns true if the bundle is an application bundle
    bool isAppBundle() const;

    // Returns true if the bundle is an AppDir
    bool isAppDir() const;

    // Returns true if the bundle is an AppImage
    bool isAppImage() const;

    // Returns true if the bundle is a .desktop file
    bool isDesktopFile() const;

    // Returns the name of the bundle
    QString name() const;

    // Returns the icon of the bundle
    QIcon icon() const;

    // Returns the name of the icon of the bundle
    QString iconName() const;

    // Returns the executable of the bundle
    QString executable() const;

    // Returns the arguments of the bundle
    QStringList arguments() const;

private:
    QString m_path;
    bool m_isValid;
    bool m_isApp;
    bool m_isAppDir;
    bool m_isAppImage;
    bool m_isDesktopFile;
    QString m_name;
    QString m_icon;
    QString m_executable;
    QStringList m_arguments;
};

#endif // APPLICATIONBUNDLE_H
