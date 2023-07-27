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

#pragma once

#include <QFileSystemModel>
#include <QByteArray>
#include "LaunchDB.h"
#include "CombinedIconCreator.h"

class CustomFileSystemModel : public QFileSystemModel
{
Q_OBJECT
public:
    explicit CustomFileSystemModel(QObject* parent = nullptr);

    QByteArray readExtendedAttribute(const QModelIndex& index, const QString& attributeName) const;

    // Public method to access data from CustomFileSystemModel because we cannot access data from QFileSystemModel directly
    QVariant fileData(const QModelIndex& index, int role) const;

    // Public method to access the "open-with" attribute
    // NOTE: Would like to do this with an index, but don't have a valid index when this gets called for unknown reasons
    // So we'll use the QFileInfo instead for now
    QString openWith(const QFileInfo& fileInfo) const;

    // Public method to access the icon coordinates
    // NOTE: Would like to do this with an index, but as above,
    // we'll use the QFileInfo instead for now
    QPoint getIconCoordinates(const QFileInfo& fileInfo) const;

private:
    // Private member variable to store "open-with" attributes.
    mutable QMap<QModelIndex, QByteArray> openWithAttributes;

    // Private member variable to store icon coordinates.
    mutable QMap<QModelIndex, QPoint> iconCoordinates;

    LaunchDB ldb;

};
