/*-
 * Copyright (c) 2022 Simon Peter <probono@puredarwin.org>
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

/* To persist the fact that QModelItemDelegate objects in a QIconView have been moved,
 * we need to update the model to reflect the new positions of the delegates.
 * In Qt, the model is responsible for storing the data that is displayed in a view.
 * This means that if we want to persist changes to the positions of QModelItemDelegate objects in a
 * QIconView, we need to update the model to reflect the new positions of the delegates.
 */

#ifndef CUSTOMITEMDELEGATE_H
#define CUSTOMITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QObject>
#include <QFileSystemModel>
#include <QAction>
#include <QMenu>

// Define the CustomItemDelegate class, which is derived from the QStyledItemDelegate class
class CustomItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    CustomItemDelegate(QObject *parent, QFileSystemModel *fileSystemModel);

    // Destructor
    ~CustomItemDelegate();

    QString displayText(const QVariant &value, const QLocale &locale) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    // Override the eventFilter() method to handle events (e.g., move) for the delegate object
    bool eventFilter(QObject *object, QEvent *event) override;

protected:
    // Override the editorEvent() function to handle right-click events for the context menu
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

private:
    // Private member variable to hold a pointer to the QFileSystemModel object
    QFileSystemModel *m_fileSystemModel;

    // We use this to flash the icon if the item was double-clicked
    bool iconShown = false;
    bool iconVisible = false;
    int flashCount = 0;

    // Context menu
    // It is generally not a good idea to create a context menu in an ItemDelegate object,
    // because the ItemDelegate object is responsible for drawing the items in the view,
    // and it is not responsible for handling user interactions with the items.
    // We should probably fix this in a future version of the application.
    QMenu menu;
};

#endif // CUSTOMITEMDELEGATE_H
