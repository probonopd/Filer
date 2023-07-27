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

#include "CustomFileSystemModel.h"
#include "ExtendedAttributes.h"
#include <QDebug>
#include "ApplicationBundle.h"


CustomFileSystemModel::CustomFileSystemModel(QObject* parent)
        : QFileSystemModel(parent)
{
    LaunchDB ldb;
}

QByteArray CustomFileSystemModel::readExtendedAttribute(const QModelIndex& index, const QString& attributeName) const
{
    if (!index.isValid() || index.column() != 0) {
        return QByteArray(); // Invalid index or not pointing to a file entry.
    }

    QString filePath = QFileSystemModel::filePath(index);
    QByteArray attributeValue;

    ExtendedAttributes ea(filePath);
    attributeValue = ea.read(attributeName);

    return attributeValue;
}

// Returns the open-with extended attribute for the given file, or the default application for the file type if the
// attribute is not set (based on information from the LaunchDB).
QString CustomFileSystemModel::openWith(const QFileInfo& fileInfo) const {
    // NOTE: Would like to do this with an index, but don't have a valid index when this gets called for unknown reasons.
    // So we'll use the QFileInfo instead for now.
    QString filePath = fileInfo.absoluteFilePath();

    // If we already have the attribute, return it
    // Try to find this file in the model
    QModelIndex index = CustomFileSystemModel::index(filePath);
    if (index.isValid() && openWithAttributes.contains(index)) {
        return openWithAttributes[index];
    }

    // Otherwise, get it from the file's extended attributes
    QString attributeValue;
    ExtendedAttributes ea(filePath);
    attributeValue = QString(ea.read("open-with"));

    // If it's empty, get it from the LaunchDB
    if (attributeValue.isEmpty()) {
        // Get it from the LaunchDB
        LaunchDB ldb;
        attributeValue = QString(ldb.applicationForFile(filePath));

    }

    if (index.isValid()) {
        // If we found it, store it in the model for future use
        qDebug() << "Updating model with open-with attribute for " << filePath << ": " << attributeValue;
        openWithAttributes[index] = attributeValue.toUtf8();
    } else {
        qDebug() << "Did not find " << filePath << " in the model. FIXME: Find the reason why this happens. Working around for now.";
        // Once we have solved this condition, we can use an index instead of a QFileInfo for this method.
    }

    return attributeValue;
}

QPoint CustomFileSystemModel::getIconCoordinates(const QFileInfo& fileInfo) const {
    QString filePath = fileInfo.absoluteFilePath();

    // If we already have the coords, return them
    QModelIndex index = CustomFileSystemModel::index(filePath);
    if (index.isValid() && iconCoordinates.contains(index)) {
        return iconCoordinates[index];
    }

    // Otherwise, get them from extended attributes
    QPoint coords = QPoint(-1, -1); // Invalid coordinates; we'll use this to indicate that we didn't find any
    ExtendedAttributes ea(filePath);
    QString coordinates = ea.read("coordinates");
    if (!coordinates.isEmpty()) {
        qDebug() << "Read coordinates from extended attributes: " << coordinates;
        QStringList coordinatesList = coordinates.split(",");
        int x = coordinatesList.at(0).toInt();
        int y = coordinatesList.at(1).toInt();
        coords = QPoint(x, y);
        if (index.isValid()) {
            // If we found it, store it in the model for future use
            qDebug() << "Updating model with coordinates for " << filePath << ": " << coords;
            iconCoordinates[index] = coords;
        } else {
            qDebug() << "Did not find " << filePath << " in the model. FIXME: Find the reason why this happens. Working around for now.";
            // Once we have solved this condition, we can use an index instead of a QFileInfo for this method.
        }
    }
    return coords;
}