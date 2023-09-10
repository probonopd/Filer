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

#ifndef CUSTOMFILESYSTEMMODEL_H
#define CUSTOMFILESYSTEMMODEL_H

#include <QFileSystemModel>
#include <QByteArray>
#include "LaunchDB.h"
#include "CustomFileIconProvider.h"

// NOTE: Qt::UserRole + 1 is already used by QFileSystemModel for the file path
static const int OpenWithRole = Qt::UserRole + 10;
static const int CanOpenRole = Qt::UserRole + 11;
static const int IsApplicationRole =  Qt::UserRole + 12;

class CustomFileSystemModel : public QFileSystemModel
{
Q_OBJECT
public:
    explicit CustomFileSystemModel(QObject* parent = nullptr);

    ~CustomFileSystemModel() override;

    QVariant data(const QModelIndex& index, int role = Qt::ToolTipRole) const override;

    QByteArray readExtendedAttribute(const QModelIndex& index, const QString& attributeName) const;

    // Public method to access data from CustomFileSystemModel because we cannot access data from QFileSystemModel directly
    QVariant fileData(const QModelIndex& index, int role) const;

    // Public method to access the "open-with" attribute
    // NOTE: Would like to do this with an index, but don't have a valid index when this gets called for unknown reasons
    // So we'll use the QFileInfo instead for now
    QString openWith(const QFileInfo& fileInfo) const;

    // This gets called when a file is dropped onto the view
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

    //  These functions are used to set the supported drag and drop actions for the model.
    Qt::DropActions supportedDropActions() const override;
    Qt::DropActions supportedDragActions() const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;

    void setPositionForIndex(const QPoint& position, const QModelIndex& index) const;
    QPoint& getPositionForIndex(const QModelIndex& index) const;

    void persistItemPositions() const;

private:
    // Private member variable to store "open-with" attributes.
    mutable QMap<QModelIndex, QByteArray> openWithAttributes;

    // Private member variable to store "open-with" attributes.
    mutable QMap<QModelIndex, QByteArray> canOpenAttributes;

    // Private member variable to store icon coordinates.
    mutable QMap<QModelIndex, QPoint> iconCoordinates;

    // Private member variable to store whether an item is an application.
    mutable QMap<QModelIndex, bool> isApplication;

    LaunchDB ldb;

    // Private method to create a bookmark file via drag and drop, e.g., from a web browser
    bool createBrowserBookmarkFile(const QMimeData *data, QString dropTargetPath) const;

    // Private helper method to make a filename safe for the filesystem, e.g., for saving bookmarks from a web browser
    QString makeFilenameSafe(const QString& input) const;

    // The following class is defined in "CustomFileIconProvider.h"; why does the compiler not see it?
    // This is because the compiler does not see the definition of the class CustomFileIconProvider
    // in the header file CustomFileIconProvider.h for some reason. Why?
    // It is defined in the same directory as this file, and the header file is included.
    CustomFileIconProvider *m_IconProvider;
    // FIXME:  Why are we getting error: unknown type name 'CustomFileIconProvider'; did you mean 'QFileIconProvider'?

};

#endif // CUSTOMFILESYSTEMMODEL_H