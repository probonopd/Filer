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

#include "CustomProxyModel.h"
#include <QIcon>
#include <QDebug>
#include <QStorageInfo>
#include <QDir>
#include "ApplicationBundle.h"
#include <QMimeData>
#include <QUrl>
#include <QFileSystemModel>
#include "Mountpoints.h"
#include "CustomFileSystemModel.h"

QSet<QString> hiddenFileNames;

CustomProxyModel::CustomProxyModel(QObject *parent)
        : QSortFilterProxyModel(parent),
          filteringEnabled(true) // Enable filtering by default
{
}

void CustomProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    QSortFilterProxyModel::setSourceModel(sourceModel);

    if (QFileSystemModel *fileSystemModel = qobject_cast<QFileSystemModel *>(sourceModel)) {
        QString rootPath = fileSystemModel->rootPath();
        QString hiddenFilePath = rootPath + "/.hidden";

        fileWatcher.addPath(hiddenFilePath);
        // Watch for changes to the .hidden file
        connect(&fileWatcher, &QFileSystemWatcher::fileChanged, this, &CustomProxyModel::handleHiddenFileChanged);
        // Also watch the parent directory, in case the .hidden file gets deleted or created
        connect(fileSystemModel, &QFileSystemModel::directoryLoaded, this, &CustomProxyModel::handleHiddenFileChanged);

        QFile hiddenFile(hiddenFilePath);
        if (hiddenFile.open(QIODevice::ReadOnly)) {
            QTextStream in(&hiddenFile);
            while (!in.atEnd()) {
                QString line = in.readLine();
                hiddenFileNames.insert(line.trimmed());
            }
            hiddenFile.close();
        }
    }
}

bool CustomProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    QString leftPath = sourceModel()->data(left, Qt::DisplayRole).toString();
    QString rightPath = sourceModel()->data(right, Qt::DisplayRole).toString();

    QString leftFullPath = sourceModel()->index(left.row(), 0, left.parent()).data(Qt::UserRole + 1).toString();

    // Check if leftFullPath is on the Desktop and if not, just return the superclasses' lessThan
    // for performance reasons
    QString parentOfLeftFullPath = QFileInfo(leftFullPath).dir().path();
    if (parentOfLeftFullPath != QDir::homePath() + "/Desktop") {
        return QSortFilterProxyModel::lessThan(left, right);
    }

    QString rightFullPath = sourceModel()->index(right.row(), 0, right.parent()).data(Qt::UserRole + 1).toString();

    // Check if the fullPaths are symbolic links and if so, resolve them
    if (QFileInfo(leftFullPath).isSymLink()) {
        leftFullPath = QFileInfo(leftFullPath).symLinkTarget();
    }
    if (QFileInfo(rightFullPath).isSymLink()) {
        rightFullPath = QFileInfo(rightFullPath).symLinkTarget();
    }

    bool leftIsDir = QFileInfo(leftFullPath).isDir();
    bool rightIsDir = QFileInfo(rightFullPath).isDir();

    if (leftIsDir && rightIsDir) {
        // qDebug() << "leftFullPath:" << leftFullPath << "rightFullPath:" << rightFullPath;

        bool leftIsMountPoint = Mountpoints::isMountpoint(leftFullPath);
        bool rightIsMountPoint = Mountpoints::isMountpoint(rightFullPath);
        // qDebug() << "leftIsMountPoint:" << leftIsMountPoint << "rightIsMountPoint:" << rightIsMountPoint;

        ApplicationBundle *leftApplicationBundle = new ApplicationBundle(leftFullPath);
        bool leftIsApplicationBundle = leftApplicationBundle->isValid();
        ApplicationBundle *rightApplicationBundle = new ApplicationBundle(rightFullPath);
        bool rightIsApplicationBundle = rightApplicationBundle->isValid();
        delete leftApplicationBundle;
        delete rightApplicationBundle;

        if ("/" == leftFullPath && "/" != rightFullPath) {
            // Root directory comes before others
            return false;
        }

        if ("/" != leftFullPath && "/" == rightFullPath) {
            // Root directory comes before others
            return true;
        }

        if (leftIsMountPoint && !rightIsMountPoint) {
            // Mount points before non-mount points
            return false;
        }

        if (rightIsMountPoint && !leftIsMountPoint) {
            // Mount points before non-mount points
            return true;
        }

        if (leftIsApplicationBundle && !rightIsApplicationBundle) {
            // Application bundles before non-application bundles
            return false;
        }

        if (rightIsApplicationBundle && !leftIsApplicationBundle) {
            // Application bundles before non-application bundles
            return true;
        }

    }

    // Fallback to default sorting
    return QSortFilterProxyModel::lessThan(left, right);
}


// https://doc.qt.io/qt-5/model-view-programming.html#inserting-dropped-data-into-a-model
// Dropped data is handled by a model's reimplementation of QAbstractItemModel::dropMimeData()
bool CustomProxyModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    // Map to source model and call its method
    QModelIndex sourceParent = mapToSource(parent);
    return sourceModel()->dropMimeData(data, action, row, column, sourceParent);
}


//  These functions are used to specify the supported drag and drop actions.

Qt::DropActions CustomProxyModel::supportedDropActions() const
{
    // Map to source model and call its method
    return sourceModel()->supportedDropActions();
}

Qt::DropActions CustomProxyModel::supportedDragActions() const
{
    // Map to source model and call its method
    return sourceModel()->supportedDragActions();
}

// https://doc.qt.io/qt-5/model-view-programming.html#enabling-drag-and-drop-for-items
Qt::ItemFlags CustomProxyModel::flags(const QModelIndex &index) const
{
    // Map to source model and call its method
    QModelIndex sourceIndex = mapToSource(index);
    return sourceModel()->flags(sourceIndex);
}

// https://doc.qt.io/qt-5/model-view-programming.html#inserting-dropped-data-into-a-model
// Models can forbid dropping on certain items,
// or depending on the dropped data, by reimplementing QAbstractItemModel::canDropMimeData().
bool CustomProxyModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    // Map to source model and call its method
    QModelIndex sourceParent = mapToSource(parent);
    return sourceModel()->canDropMimeData(data, action, row, column, sourceParent);
}


bool CustomProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (!filteringEnabled) {
        // If filtering is disabled, accept all rows
        return true;
    }

    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    QString fileName = static_cast<QFileSystemModel *>(sourceModel())->fileName(index);

    // Filter out files starting with a dot
    if (fileName.startsWith('.')) {
        return false; // Do not accept this row
    }

    if (hiddenFileNames.contains(fileName.trimmed())) {
        return false; // Do not accept this row
    }

    return true; // Accept this row
}

void CustomProxyModel::handleHiddenFileChanged(const QString &path)
{
    Q_UNUSED(path);
    updateFiltering();
}

void CustomProxyModel::loadHiddenFileNames(const QString &hiddenFilePath)
{
    QFile hiddenFile(hiddenFilePath);
    if (hiddenFile.open(QIODevice::ReadOnly)) {
        QTextStream in(&hiddenFile);
        while (!in.atEnd()) {
            QString line = in.readLine();
            hiddenFileNames.insert(line.trimmed());
        }
        hiddenFile.close();
    }
}

void CustomProxyModel::updateFiltering()
{
    hiddenFileNames.clear();
    if (QFileSystemModel *fileSystemModel = qobject_cast<QFileSystemModel *>(sourceModel())) {
        QString rootPath = fileSystemModel->rootPath();
        QString hiddenFilePath = rootPath + "/.hidden";
        loadHiddenFileNames(hiddenFilePath);
    }

    invalidateFilter();
}

void CustomProxyModel::setFilteringEnabled(bool enable)
{
    if (filteringEnabled != enable) {
        filteringEnabled = enable;
        updateFiltering();
    }
}

bool CustomProxyModel::isFilteringEnabled() const
{
    return filteringEnabled;
}