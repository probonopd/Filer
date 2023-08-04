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

/**
 * @file DesktopFile.h
 * @brief The DesktopFile class provides functionality to work with desktop entry files.
 *
 * Note: This class provides minimal support for XDG desktop files for backward compatibility
 * with legacy applications. Consider bundle formats like .app, .AppDir, or .AppImage instead,
 * as Filer is optimized for these.
 */

#ifndef DESKTOPFILE_H
#define DESKTOPFILE_H

#include <QString>

/**
 * @file DesktopFile.h
 * @brief The DesktopFile class provides functionality to work with desktop entry files.
 *
 * Note: This class provides minimal support for XDG desktop files for backward compatibility
 * with legacy applications. Consider bundle formats like .app, .AppDir, or .AppImage instead,
 * as Filer is optimized for these.
 */
class DesktopFile
{
public:
    /**
     * @brief Retrieves the value for Icon= from a desktop file.
     * @param filename The path of the desktop entry file to work with.
     * @return The name of the icon, or an empty string if not found.
     */
    static QString getIcon(const QString &filename);

    /**
     * @brief Retrieves the value for Name= from a desktop file.
     * @param filename The path of the desktop entry file to work with.
     * @return The name of the desktop entry, or an empty string if not found.
     */
    static QString getName(const QString &filename);

    /**
     * @brief Retrieves the value of a key from a desktop file.
     * @param filename The path of the desktop entry file to work with.
     * @param key The key to look for.
     * @return The value of the key, or an empty string if not found.
     */
    static QString getValue(const QString &filename, const QString &key);

private:
    DesktopFile(); /**< Private constructor to prevent instantiation. */
};

#endif // DESKTOPFILE_H
