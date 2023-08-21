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

CustomProxyModel::CustomProxyModel(QObject *parent)
        : QSortFilterProxyModel(parent)
{
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

        QStringList mountPoints;
        for (const QStorageInfo &storage : QStorageInfo::mountedVolumes()) {
            mountPoints << storage.rootPath();
        }

        bool leftIsMountPoint = mountPoints.contains(leftFullPath);
        bool rightIsMountPoint = mountPoints.contains(rightFullPath);
        // qDebug() << "leftIsMountPoint:" << leftIsMountPoint << "rightIsMountPoint:" << rightIsMountPoint;

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
    }

    // Fallback to default sorting
    return QSortFilterProxyModel::lessThan(left, right);
}
