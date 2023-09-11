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
#include "DBusInterface.h"
#include "FileOperationManager.h"
#include "DragAndDropHandler.h"
#include "AppGlobals.h"
#include <QSettings>
#include <QScrollBar>
#include <QStandardPaths>
#include <QShortcut>

CustomListView::CustomListView(QWidget* parent) : QListView(parent) {

    // Before blocking updates, paint the desktop picture; does this work?
    if (should_paint_desktop_picture) {
        paintDesktopPicture();
    }

    // Block updating the view until all items have been moved to their custom positions in CustomListView::layoutItems()
    this->viewport()->setUpdatesEnabled(false);

    // Single click to rename; this requires the item to have the Qt::ItemIsEditable flag set
    setEditTriggers(QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);

    // Make items freely movable
    setMovement(QListView::Free);
    // This alone is not sufficient; dropEvent of the view (superclass) also needs to be called

    // Do not relayout while the window is being resized
    setResizeMode(QListView::Fixed); // "The items will only be laid out the first time the view is shown"
    // For this to work properly, we seemingly need to set item positions
    // using CustomFileSystemModel::setPositionForIndex before the view is resized
    // so that the positions are known while the view is resized

    should_paint_desktop_picture = false;

    // Allow drag and drop
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDefaultDropAction(Qt::MoveAction);

    DragAndDropHandler *handler = new DragAndDropHandler(this);

    connect(this, &CustomListView::dragEnterEventSignal, handler, &DragAndDropHandler::handleDragEnterEvent);
    connect(this, &CustomListView::dragMoveEventSignal, handler, &DragAndDropHandler::handleDragMoveEvent);
    connect(this, &CustomListView::dropEventSignal, handler, &DragAndDropHandler::handleDropEvent);
    connect(this, &CustomListView::dragLeaveEventSignal, handler, &DragAndDropHandler::handleDragLeaveEvent);

    m_layoutTimer = new QTimer();
    m_layoutTimer->setSingleShot(true);
    connect(m_layoutTimer, &QTimer::timeout, this, &CustomListView::layoutItems);

    m_sourceModel = this->model();
    // m_sourceModel = m_proxyModel->sourceModel();

}

CustomListView::~CustomListView() {
    qDebug() << "CustomListView::~CustomListView";
}

void CustomListView::requestDesktopPictureToBePainted(bool request) {
    qDebug() << "CustomListView::requestDesktopPictureToBePainted" << request;
    should_paint_desktop_picture = request;
}

void CustomListView::paintDesktopPicture()
{

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

}

void CustomListView::paintEvent(QPaintEvent* event)
{

    if(!should_paint_desktop_picture) {
        QListView::paintEvent(event);
        return;
    }

    paintDesktopPicture();

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
    // emit startDragSignal(supportedActions);

    // Superclass implementation
    QListView::startDrag(supportedActions);
}

void CustomListView::layoutItems() {

    int max_x = 0;
    int max_y = 0;
    int max_width = 0;
    int max_height = 0;

    qDebug() << "CustomListView::layoutItems";
    if (m_layoutTimer->isActive()) {
        m_layoutTimer->stop();
    }

    QAbstractProxyModel* model = qobject_cast<QAbstractProxyModel*>(this->model());
    qDebug() << "CustomListView::layoutItems() model" << model;
    CustomFileSystemModel* sourceModel = qobject_cast<CustomFileSystemModel*>(model->sourceModel());
    qDebug() << "CustomListView::layoutItems() sourceModel" << sourceModel;
    QModelIndex rootIndex = this->rootIndex();
    qDebug() << "CustomListView::layoutItems() rootIndex" << rootIndex;
    QString rootPath = model->data(rootIndex, QFileSystemModel::FilePathRole).toString();
    qDebug() << "CustomListView::layoutItems() rootPath" << rootPath;

    int itemCount = model->rowCount(rootIndex);
    qDebug() << "CustomListView::layoutItems() itemCount" << itemCount;
    for (int row = 0; row < itemCount; ++row) {
        QModelIndex index = model->index(row, 0, rootIndex);
        QModelIndex sourceIndex = model->mapToSource(index);
        // qDebug() << "CustomListView::layoutItems() index" << index;
        // Print the name of the item
        // QString path = model->data(index, Qt::DisplayRole).toString();
        // qDebug() << "CustomListView::layoutItems() path" << path;

        // If needed, we could use getItemDelegateForIndex
        // QAbstractItemDelegate* delegate = this->getItemDelegateForIndex(index);
        // qDebug() << "CustomListView::layoutItems() delegate" << delegate;
        // We could also cast it to CustomItemDelegate if we really needed to
        // but for now we don't need to. Maybe we will need this later when we want to
        // get the size of the item, for example.

        int x = -1;
        int y = -1;

        QPoint position = sourceModel->getPositionForIndex(sourceIndex);
        if (position != QPoint(-1, -1)) {
            // qDebug() << "CustomListView::layoutItems() position" << position;
            x = position.x();
            y = position.y();
            max_x = qMax(max_x, x);
            max_y = qMax(max_y, y);
            // TODO: If we are rendering the desktop and the item is outside the window, move it inside
            setPositionForIndex(QPoint(x, y), index); // Actually move it
            // Update iconCoordinates[index] which will get persisted to disk when the window closes
            sourceModel->setPositionForIndex(QPoint(x, y), sourceIndex);
        } else {
            // Randomize within the whole window
            // x = qrand() % (this->width() - 100);
            // y = qrand() % (this->height() - 100);
        }

        // TODO: Align the items to make efficient use of the space
        // Calculate positions based on the width and height of the items
        // and the width and height of the window, so that we can have the items aligned without
        // being too close to each other, and without having too much space between them.
        // So no real grid, but a more dynamic layout.
    }

    // Print the maximum x and y values
    qDebug() << "CustomListView::layoutItems() max_x" << max_x;
    qDebug() << "CustomListView::layoutItems() max_y" << max_y;

    // Print scroll bar range
    qDebug() << "CustomListView::layoutItems() horizontalScrollBar()->minimum()" << this->horizontalScrollBar()->minimum();
    qDebug() << "CustomListView::layoutItems() horizontalScrollBar()->maximum()" << this->horizontalScrollBar()->maximum();

    // Set scroll bar range if we are not rendering the desktop
    // FIXME: Calculate the range based on the size of the items
    // Maybe we need the delegates for this?
    // This is probably not correct, but it seems to give a usable approximation
    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    if (rootPath != desktopPath) {
        this->horizontalScrollBar()->setRange(0, max_x-100);
        this->verticalScrollBar()->setRange(0, max_y-200);
    }

    // Undo "Block updating the view until all items have been moved to their custom positions in CustomListView::layoutItems()"
    // In 10 ms, we call this->viewport()->setUpdatesEnabled(false);
    // FIXME: Do not hardcode a particular delay, but get called whenever the view is ready to be updated
    QTimer::singleShot(10, [this] () { this->viewport()->setUpdatesEnabled(true); });
}

void CustomListView::queueLayout(int delay) {
   // qDebug() << "CustomListView::queueLayout" << delay;
    m_layoutTimer->stop();
    m_layoutTimer->start(delay);
}

void CustomListView::resizeEvent(QResizeEvent* event) {
    qDebug() << "CustomListView::resizeEvent";
    // Ignore the event, because we don't want to layout the items immediately
    // FIXME: This is not working yet
    event->ignore();
    // queueLayout(100);
}

void CustomListView::updateGeometries() {
    qDebug() << "CustomListView::updateGeometries";
    // QListView::updateGeometries();
    queueLayout(0);
}

// Make superclass' dropEvent(QDropEvent *event) public
// This is used to draw items at the correct position after a drop
// if they were just moved in the same view
void CustomListView::specialDropEvent(QDropEvent *event) {
    QListView::dropEvent(event);

    // Print all the model indexes of the dropped items
    QAbstractItemModel* model = this->model();
    qDebug() << "CustomListView::specialDropEvent() model" << model;
    QModelIndexList indexes = this->selectedIndexes();
    qDebug() << "CustomListView::specialDropEvent() indexes" << indexes;

            foreach (QModelIndex index, indexes) {
            qDebug() << "CustomListView::specialDropEvent() index" << index;

            // Print the name of the item
            // QString path = model->data(index, Qt::DisplayRole).toString();
            // qDebug() << "CustomListView::specialDropEvent() path" << path;

            // Map the index to the source model
            QAbstractProxyModel* proxyModel = qobject_cast<QAbstractProxyModel*>(model);
            qDebug() << "CustomListView::specialDropEvent() proxyModel" << proxyModel;
            QModelIndex sourceIndex = proxyModel->mapToSource(index);
            CustomFileSystemModel* sourceModel = qobject_cast<CustomFileSystemModel*>(proxyModel->sourceModel());
            qDebug() << "CustomListView::specialDropEvent() sourceIndex" << sourceIndex;

            // Map the position to global coordinates
            // NOTE: The coordinates for items in a QListView or similar views are specified using a QRect.
            // A QRect is a rectangle defined by its top-left corner (x, y) and its width and height.
            QPoint globalPos = this->mapToGlobal(this->visualRect(index).topLeft());

            // Convert global coordinates to local coordinates
            QPoint position = this->mapFromGlobal(globalPos);

            qDebug() << "CustomListView::specialDropEvent() position" << position;
            sourceModel->setPositionForIndex(position, sourceIndex);
            qDebug() << "CustomListView::specialDropEvent() sourceModel->setPositionForIndex(sourceIndex)";

        }
}
