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
 * @file ApplicationBundle.h
 * @brief Definition of the ApplicationBundle class.
 */

#ifndef APPLICATIONBUNDLE_H
#define APPLICATIONBUNDLE_H

#include <QString>
#include <QStringList>
#include <QIcon>
#include <QObject>

/**
 * @class ApplicationBundle
 * @brief Represents an application bundle or executable in various formats.
 *
 * The ApplicationBundle class provides methods to work with different types of application
 * bundles, including AppImages, AppDirs, desktop files, and regular executables.
 * It can be used to retrieve information about the application, such as its type, icon,
 * name, executable path, and arguments.
 *
 * The class supports multiple types of application bundles, each identified by its Type.
 * Application types include AppBundle (application bundle directory), AppDir (AppImage-like),
 * AppImage (AppImage executable), and DesktopFile (desktop entry).
 */
class ApplicationBundle : public QObject {
Q_OBJECT

public:
    /**
     * @brief Enumerates the different types of application bundles.
     */
    enum class Type {
        Unknown,       /**< Unknown application bundle type. */
        AppBundle,     /**< Simplified .app bundle. */
        AppDir,        /**< ROX-style AppDir. */
        AppImage,      /**< AppImage executable. */
        DesktopFile    /**< Desktop entry file. */
    };

    /**
     * @brief Constructs an ApplicationBundle object for the specified path.
     * @param path The path to the application bundle.
     */
    ApplicationBundle(const QString& path);

    /**
     * @brief Returns the path of the application bundle.
     * @return The path.
     */
    QString path() const;

    /**
     * @brief Checks if the application bundle is valid and exists.
     * @return True if the bundle is valid, false otherwise.
     */
    bool isValid() const;

    /**
     * @brief Returns the type of the application bundle.
     * @return The type.
     */
    Type type() const;

    /**
     * @brief Retrieves the icon of the application bundle.
     * @return The icon.
     */
    QIcon icon() const;

    /**
     * @brief Retrieves the name of the icon file.
     * @return The icon name.
     */
    QString iconName() const;

    /**
     * @brief Retrieves the name of the application.
     * @return The name.
     */
    QString name() const;

    /**
     * @brief Retrieves the path to the executable of the application.
     * @return The executable path.
     */
    QString executable() const;

    /**
     * @brief Retrieves the list of arguments passed to the executable.
     * @return The list of arguments.
     */
    QStringList arguments() const;

    /**
     * @brief Checks if the application bundle is a command line tool.
     * @return True if the application is a command-line tool, false otherwise.
     */
    bool isCommandLineTool() const;

    /**
     * @brief Gets the name of the type as a string.
     * @return The type name.
     */
    QString typeName() const;

    /**
     * @brief Launches the application.
     * @param arguments The list of arguments to pass to the application.
     * @return True if the application was launched successfully, false otherwise.
     */
    bool launch(QStringList arguments) const;

QString m_path;         /**< The path of the application bundle. */
    bool m_isValid;         /**< Indicates if the bundle is valid. */
    Type m_type;            /**< The type of the application bundle. */
    QString m_name;         /**< The name of the application. */
    QString m_icon;         /**< The path of the application's icon. */
    QString m_executable;   /**< The path to the executable. */
    QStringList m_arguments;/**< The list of arguments. */
    QIcon quadraticIcon(QIcon icon) const;
};

#endif // APPLICATIONBUNDLE_H

