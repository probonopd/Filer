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

#ifndef DBUSINTERFACE_H
#define DBUSINTERFACE_H

#include <QObject>

/**
 * @brief The DBusInterface class represents the D-Bus interface for file management operations.
 */
class DBusInterface : public QObject
{
Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.FileManager1")

public:
    /**
     * @brief Constructs a DBusInterface object.
     */
    DBusInterface();

    /**
     * @brief Shows folders corresponding to the provided URIs.
     * @param uriList List of URIs representing folders to be shown.
     * @param startUpId Identifier for startup.
     */
    Q_SCRIPTABLE void ShowFolders(const QStringList &uriList, const QString &startUpId);

    /**
     * @brief Shows items corresponding to the provided URIs.
     * @param uriList List of URIs representing items to be shown.
     * @param startUpId Identifier for startup.
     */
    Q_SCRIPTABLE void ShowItems(const QStringList &uriList, const QString &startUpId);

    /**
     * @brief Shows properties of items corresponding to the provided URIs.
     * @param uriList List of URIs representing items for which properties are to be shown.
     * @param startUpId Identifier for startup.
     */
    Q_SCRIPTABLE void ShowItemProperties(const QStringList &uriList, const QString &startUpId);

    /**
     * @brief Retrieves the sort order for a given URL.
     * @param url The URL for which the sort order is to be retrieved.
     * @param role The role for sorting.
     * @param order The sorting order.
     */
    Q_SCRIPTABLE void SortOrderForUrl(const QString &url, QString &role, QString &order);

private:
    bool m_isDaemon = false; /**< Indicates if the interface is used as a daemon. */
};

#endif // DBUSINTERFACE_H
