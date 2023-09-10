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

#include <QTreeView>
#include "CustomTreeView.h"
#include "DragAndDropHandler.h"

CustomTreeView::CustomTreeView(QWidget* parent) : QTreeView(parent) {

    // Allow sorting by clicking on the column headers
    setSortingEnabled(true);

    // Single click to rename; this requires the item to have the Qt::ItemIsEditable flag set
    setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);

    DragAndDropHandler *handler = new DragAndDropHandler(this);
    connect(this, &CustomTreeView::dragEnterEventSignal, handler, &DragAndDropHandler::handleDragEnterEvent);
    connect(this, &CustomTreeView::dragMoveEventSignal, handler, &DragAndDropHandler::handleDragMoveEvent);
    connect(this, &CustomTreeView::dropEventSignal, handler, &DragAndDropHandler::handleDropEvent);
    connect(this, &CustomTreeView::dragLeaveEventSignal, handler, &DragAndDropHandler::handleDragLeaveEvent);
}

void CustomTreeView::dragEnterEvent(QDragEnterEvent *event) {
    // We handle this in the DragAndDropHandler, so we emit a signal here
    // which is connected to the DragAndDropHandler
    emit dragEnterEventSignal(event);
}

void CustomTreeView::dragMoveEvent(QDragMoveEvent *event) {
    // We handle this in the DragAndDropHandler, so we emit a signal here
    // which is connected to the DragAndDropHandler
    emit dragMoveEventSignal(event);
}

void CustomTreeView::dragLeaveEvent(QDragLeaveEvent *event) {
    // We handle this in the DragAndDropHandler, so we emit a signal here
    // which is connected to the DragAndDropHandler
    emit dragLeaveEventSignal(event);
}

void CustomTreeView::dropEvent(QDropEvent *event) {
    // We handle this in the DragAndDropHandler, so we emit a signal here
    // which is connected to the DragAndDropHandler
    emit dropEventSignal(event);
}

void CustomTreeView::startDrag(Qt::DropActions supportedActions) {
    // We handle this in the DragAndDropHandler, so we emit a signal here
    // which is connected to the DragAndDropHandler
    emit startDragSignal(supportedActions);
}