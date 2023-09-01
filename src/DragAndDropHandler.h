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

#ifndef DRAGANDDROPHANDLER_H
#define DRAGANDDROPHANDLER_H

#include <QObject>
#include <QAbstractItemView>
#include <QTimer>

// This is a friend class of QAbstractItemView
class QAbstractItemView;

/**
 * @file DragAndDropHandler.h
 * @class DragAndDropHandler
 * @brief Handles drag and drop events for a QAbstractItemView so that we don't have to implement the same
 * code in multiple views.
 * It is a friend class of QAbstractItemView.
 */
class DragAndDropHandler : public QObject {
Q_OBJECT
public:
    /**
     * @brief Constructs a DragAndDropHandler with the given QAbstractItemView and parent object.
     *
     * @param view The QAbstractItemView for which drag and drop events should be handled.
     * @param parent The parent QObject for this handler (optional).
     */
    explicit DragAndDropHandler(QAbstractItemView *view, QObject *parent = nullptr);

public slots:
    /**
     * @brief Handles the drag enter event.
     *
     * @param event The QDragEnterEvent containing information about the drag and drop operation.
     */
    void handleDragEnterEvent(QDragEnterEvent* event);

    /**
     * @brief Handles the drag move event.
     *
     * @param event The QDragMoveEvent containing information about the drag and drop operation.
     */
    void handleDragMoveEvent(QDragMoveEvent* event);

    /**
     * @brief Handles the drop event.
     *
     * @param event The QDropEvent containing information about the drag and drop operation.
     */
    void handleDropEvent(QDropEvent* event);

    /**
     * @brief Handles the drag leave event.
     *
     * @param event The QDragLeaveEvent containing information about the drag and drop operation.
     */
    void handleDragLeaveEvent(QDragLeaveEvent *event);

    /**
     * @brief Handles the start drag event.
     *
     * @param supportedActions The supported drag actions.
     */
    void handleStartDrag(Qt::DropActions supportedActions);

private slots:
    /**
     * @brief springLoad
     * @param index
     * @param delay
     * This slot is called when the mouse hovers over an item for a while.
     * It expands the item if it is a folder.
     */
    void mouseHoversOver();

private:
    QAbstractItemView *m_view;

    // For spring-loaded folders
    QTimer m_springTimer;
    QModelIndex m_potentialTargetIndex;
};

#endif // DRAGANDDROPHANDLER_H
