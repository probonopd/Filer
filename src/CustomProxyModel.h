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

#ifndef CUSTOMPROXYMODEL_H
#define CUSTOMPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QModelIndex>

/**
 * @file CustomProxyModel.h
 * @class CustomProxyModel
 * @brief A custom proxy model for sorting items in views.
 *
 * The CustomProxyModel class provides a custom proxy model for sorting items in views,
 * such as QTreeView or QListView. It extends QSortFilterProxyModel and adds custom behavior
 * like sorting mount points before non-mount points on the desktop.
 */
class CustomProxyModel : public QSortFilterProxyModel
{
Q_OBJECT

public:
    /**
     * @brief Constructs a CustomProxyModel object.
     * @param parent The parent QObject.
     */
    CustomProxyModel(QObject *parent = nullptr);

    /**
     * @brief Returns whether the item referred to by the given index is less than the item
     *        referred to by the given other index. Used for sorting. Sorts mount points
     *        before non-mount points.
     * @param left The left index.
     * @param right The right index.
     * @return True if the item referred to by the left index is less than the item referred
     *         to by the right index; otherwise returns false. Mount points come before
     *         non-mount points on the desktop.
     */
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

    // This gets called when a file is dropped onto the view
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

    //  These functions are used to set the supported drag and drop actions for the model.
    Qt::DropActions supportedDropActions() const override;
    Qt::DropActions supportedDragActions() const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;

};

#endif // CUSTOMPROXYMODEL_H
