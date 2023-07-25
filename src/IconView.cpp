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

#include "iconview.h"

#include <QAbstractItemModel>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QPaintEvent>

IconView::IconView()
{
    // Construct the IconView object
}

void IconView::paintEvent(QPaintEvent *event)
{
    // Call the base class implementation of paintEvent() to paint the view
    QListView::paintEvent(event);

    // Get the model associated with the view
    QAbstractItemModel *model = this->model();

    // Loop through all rows in the model
    for (int row = 0; row < model->rowCount(); row++) {
        // Get the index for the current row
        QModelIndex index = model->index(row, 0);

        // Get the item delegate for the current index
        QAbstractItemDelegate *delegate = this->itemDelegate(index);

        qDebug() << "TODO: Restore position for file:" << index.data(Qt::DisplayRole).toString();

        // Save the current position of the delegate in the model
        model->setData(index, delegate->pos(), Qt::UserRole);
    }
}

void IconView::dragMoveEvent(QDragMoveEvent *event)
{
    // Call the base class implementation of dragMoveEvent() to handle the event
    QListView::dragMoveEvent(event);

    // Update the model to reflect the new position of the delegate
    QModelIndex index = this->indexAt(event->pos());
    this->model()->setData(index, event->pos(), Qt::UserRole);
    qDebug() << "TODO: Save position for file:" << index.data(Qt::DisplayRole).toString();
    qDebug() << "Position:" << event->pos();
}

void IconView::dropEvent(QDropEvent *event)
{
    // Call the base class implementation of dropEvent() to handle the event
    QListView::dropEvent(event);

    // Update the model to reflect the new position of the delegate
    QModelIndex index = this->indexAt(event->pos());
    this->model()->setData(index, event->pos(), Qt::UserRole);
    qDebug() << "TODO: Save position for file:" << index.data(Qt::DisplayRole).toString();
    qDebug() << "Position:" << event->pos();
}
