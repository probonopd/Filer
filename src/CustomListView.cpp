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

#include "CustomListView.h"
#include <QMimeData>
#include <QFileSystemModel>
#include <QAbstractProxyModel>
#include "CustomFileSystemModel.h"
#include "FileManagerMainWindow.h"
#include "ApplicationBundle.h"
#include <QApplication>
#include "CustomProxyModel.h"
#include "DBusInterface.h"
#include "FileOperationManager.h"
#include "DragAndDropHandler.h"
#include "AppGlobals.h"
#include <QSettings>

CustomListView::CustomListView(QWidget* parent) : QListView(parent) {
    should_paint_desktop_picture = false;

    DragAndDropHandler *handler = new DragAndDropHandler(this);
    connect(this, &CustomListView::dragEnterEventSignal, handler, &DragAndDropHandler::handleDragEnterEvent);
    connect(this, &CustomListView::dragMoveEventSignal, handler, &DragAndDropHandler::handleDragMoveEvent);
    connect(this, &CustomListView::dropEventSignal, handler, &DragAndDropHandler::handleDropEvent);
    connect(this, &CustomListView::dragLeaveEventSignal, handler, &DragAndDropHandler::handleDragLeaveEvent);
    connect(this, &CustomListView::startDragSignal, handler, &DragAndDropHandler::handleStartDrag);
}

CustomListView::~CustomListView() {
    qDebug() << "CustomListView::~CustomListView";
}

void CustomListView::requestDesktopPictureToBePainted(bool request) {
    qDebug() << "CustomListView::requestDesktopPictureToBePainted" << request;
    should_paint_desktop_picture = request;
}

void CustomListView::paintEvent(QPaintEvent* event)
{

    if(!should_paint_desktop_picture) {
        QListView::paintEvent(event);
        return;
    }

    QPainter painter(viewport());

    // Save the painter state
    painter.save();

    // From QSettings, get the value for desktopPicture; if not set, use the default
    QString desktopPicturePath = QSettings().value("desktopPicture", "/usr/local/share/slim/themes/default/background.jpg").toString();
    // If exists, use the user's desktop picture
    if (QFile::exists(desktopPicturePath)) {
        // Draw the desktop picture
        QPixmap background(desktopPicturePath);
        background = background.scaled(this->size(), Qt::KeepAspectRatioByExpanding);
        painter.drawPixmap(0, 0, background);

        // Draw a grey background over it to make it more muted; TODO: Remove this and fix the desktop picture instead
        painter.fillRect(this->rect(), QColor(128, 128, 128, 128));
    } else {
        // If not, use a solid color gradient
        QLinearGradient gradient(0, 0, 0, this->height());
        gradient.setColorAt(0, QColor(128-30, 128, 128+30));
        gradient.setColorAt(1, QColor(48-30, 48, 48+30));
        painter.fillRect(this->rect(), gradient);
    }

    // Draw a rectangle with a gradient at the top of the window
    // so that the Menu is more visible
    QPen pen(Qt::NoPen);
    painter.setPen(pen);
    QRect rect(0, 0, this->width(), 44);
    QLinearGradient gradient(0, 0, 0, 22);
    gradient.setColorAt(0, QColor(0, 0, 0, 50));
    gradient.setColorAt(1, QColor(0, 0, 0, 0));
    painter.fillRect(rect, gradient);

    // Restore the painter state
    painter.restore();

    // Call super class paintEvent to draw the items
    QListView::paintEvent(event);
}

void CustomListView::dragEnterEvent(QDragEnterEvent *event) {
    // We handle this in the DragAndDropHandler, so we emit a signal here
    // which is connected to the DragAndDropHandler
    emit dragEnterEventSignal(event);
}

void CustomListView::dragMoveEvent(QDragMoveEvent *event) {
    // We handle this in the DragAndDropHandler, so we emit a signal here
    // which is connected to the DragAndDropHandler
    emit dragMoveEventSignal(event);
}

void CustomListView::dragLeaveEvent(QDragLeaveEvent *event) {
    // We handle this in the DragAndDropHandler, so we emit a signal here
    // which is connected to the DragAndDropHandler
    emit dragLeaveEventSignal(event);
}

void CustomListView::dropEvent(QDropEvent *event) {
    // We handle this in the DragAndDropHandler, so we emit a signal here
    // which is connected to the DragAndDropHandler
    emit dropEventSignal(event);
}

void CustomListView::startDrag(Qt::DropActions supportedActions) {
    // We handle this in the DragAndDropHandler, so we emit a signal here
    // which is connected to the DragAndDropHandler
   emit startDragSignal(supportedActions);
}