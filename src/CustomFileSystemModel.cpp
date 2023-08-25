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
#include <QMimeData>
#include <QUrl>
#include <QMessageBox>

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
        // qDebug() << "Updating model with open-with attribute for " << filePath << ": " << attributeValue;
        openWithAttributes[index] = attributeValue.toUtf8();
    } else {
        // qDebug() << "Did not find " << filePath << " in the model. FIXME: Find the reason why this happens. Working around for now.";
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
            // qDebug() << "Did not find " << filePath << " in the model. FIXME: Find the reason why this happens. Working around for now.";
            // Once we have solved this condition, we can use an index instead of a QFileInfo for this method.
        }
    }
    return coords;
}

// https://doc.qt.io/qt-5/model-view-programming.html#inserting-dropped-data-into-a-model
// Dropped data is handled by a model's reimplementation of QAbstractItemModel::dropMimeData()
bool CustomFileSystemModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    qDebug() << "CustomFileSystemModel::dropMimeData action" << action;
    qDebug() << "TODO: Implement the actual filesystem operations in dropMimeData()";

    // https://doc.qt.io/qt-5/model-view-programming.html#inserting-dropped-data-into-a-model
    // When a drop occurs, the model index corresponding to the parent item will either be valid,
    // indicating that the drop occurred on an item, or it will be invalid,
    // indicating that the drop occurred somewhere in the view that corresponds to top level of the model.
    QModelIndex index = parent;
    QString dropTargetPath;
    if (!index.isValid()) {
        qDebug() << "CustomFileSystemModel::dropMimeData Drop occurred on top level of the model (not on an item)";
        dropTargetPath = rootPath();
        qDebug() << "CustomFileSystemModel::dropMimeData dropTargetPath:" << dropTargetPath;
    } else {
        qDebug() << "CustomFileSystemModel::dropMimeData Drop occurred on an item, TODO: Handle in item delegate";
        // TODO: Handle this case in the item delegate since finding out which item was dropped on is not trivial here
    }

    if (data->hasUrls()) {
        QList<QUrl> urls = data->urls();
        for (int i = 0; i < urls.size(); ++i) {
            if (urls.at(i).isLocalFile()) {
                qDebug() << "CustomFileSystemModel::dropMimeData Dropped file" << urls.at(i).toLocalFile();
            } else {
                qDebug() << "CustomFileSystemModel::dropMimeData Dropped URL" << urls.at(i).toString();
                int success = createBrowserBookmarkFile(data, dropTargetPath);
                if (success) {
                    qDebug() << "CustomFileSystemModel::dropMimeData Successfully created bookmark file";
                } else {
                    QMessageBox::warning(0, 0, tr("Could not create bookmark file"));
                }
            }
        }

        // Emit signals to update the view if needed
        // ...

        return true; // Signal that the drop operation was successful
    }

    return false; // Signal that the drop operation failed
}


//  These functions are used to specify the supported drag and drop actions.

Qt::DropActions CustomFileSystemModel::supportedDropActions() const
{
    qDebug() << "CustomFileSystemModel::supportedDropActions";
    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

Qt::DropActions CustomFileSystemModel::supportedDragActions() const
{
    qDebug() << "CustomFileSystemModel::supportedDragActions";
    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

// https://doc.qt.io/qt-5/model-view-programming.html#enabling-drag-and-drop-for-items
Qt::ItemFlags CustomFileSystemModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QFileSystemModel::flags(index);

    if (index.isValid())
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    else
        return Qt::ItemIsDropEnabled | defaultFlags;
}

// https://doc.qt.io/qt-5/model-view-programming.html#inserting-dropped-data-into-a-model
// Models can forbid dropping on certain items,
// or depending on the dropped data, by reimplementing QAbstractItemModel::canDropMimeData().
bool CustomFileSystemModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{

    qDebug() << "CustomFileSystemModel::canDropMimeData";

    if (data->hasUrls()) {
        QList<QUrl> urls = data->urls();
        for (int i = 0; i < urls.size(); ++i) {
            QString filePath = urls.at(i).toLocalFile();
            qDebug() << "CustomFileSystemModel::canDropMimeData Dragged file" << filePath;
        }

        // Emit signals to update the view if needed
        // ...

        return true; // Signal that the drop operation was successful
    }

    return false; // Signal that the drop operation failed
}

QString CustomFileSystemModel::makeFilenameSafe(const QString& input) const
{
    QString output = input;
    // Convert the string to lowercase
    output = output.toLower();
    // Replace all non-alphanumeric characters with a hyphen
    output = output.replace(QRegExp("[^a-z0-9]"), "-");
    // Remove any leading or trailing hyphens
    output = output.replace(QRegExp("^-"), "");
    output = output.replace(QRegExp("-$"), "");
    return output;
}

bool CustomFileSystemModel::createBrowserBookmarkFile(const QMimeData *data, QString dropTargetPath) const {
    qDebug() << "CustomFileSystemModel::createBrowserBookmarkFile" << dropTargetPath;
    QString titleText = "Link";
    if(data->hasText()) {
        titleText = data->text();
    }

    QFile file(dropTargetPath + "/" + makeFilenameSafe(titleText) + ".desktop");
    qDebug() << "CustomFileSystemModel::createBrowserBookmarkFile" << file.fileName();
    QString titleTextShort = titleText.split("/").last();
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << "[Desktop Entry]\n";
        stream << "Name=" + titleTextShort + "\n";
        stream << "Exec=open " << "\"" + data->urls().first().toString(QUrl::FullyEncoded) + "\"\n";
        // stream << "URL=" << "\"" + data->urls().first().toString(QUrl::FullyEncoded) + "\"\n";
        stream << "Type=Application\n";
        stream << "Icon=gnome-globe\n";
        file.close();
        // chmod 0755 equivalent
        file.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                            QFile::ReadGroup | QFile::ExeGroup |
                            QFile::ReadOther | QFile::ExeOther);
        qDebug() << "CustomFileSystemModel::createBrowserBookmarkFile Successfully created bookmark file";
        return true;
    }

    return false;
}