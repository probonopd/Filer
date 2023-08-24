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

CustomListView::CustomListView(QWidget* parent) : QListView(parent) {
    should_paint_desktop_picture = false;

    // https://doc.qt.io/qt-5/model-view-programming.html#using-convenience-views
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragEnabled(true);
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);
    // Enable the user to move the items around within the view
    setDragDropMode(QAbstractItemView::InternalMove);
    // NOTE: the model used also has to provide support for drag and drop operations.
    // The actions supported by a model can be specified by reimplementing the
    // QAbstractItemModel::supportedDropActions() function.
    // NOTE: Enabling drag and drop for items
    // Models indicate to views which items can be dragged, and which will accept drops,
    // by reimplementing the QAbstractItemModel::flags() function to provide suitable flags.

    // Spring-loaded folders
    connect(&m_springTimer, &QTimer::timeout, this, &CustomListView::expandTarget);
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

    QString desktopPicture = "/usr/local/share/slim/themes/default/background.jpg";

    // If exists, use the user's desktop picture
    if (QFile::exists(desktopPicture)) {
        // Draw the desktop picture
        QPixmap background(desktopPicture);
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

void CustomListView::dragEnterEvent(QDragEnterEvent* event)
{
    qDebug() << "CustomListView::dragEnterEvent";
    qDebug() << "CustomListView::dragEnterEvent event->proposedAction()" << event->proposedAction();

    if (event->mimeData()->hasFormat("text/uri-list")) {
        event->acceptProposedAction();
        qDebug() << "CustomListView::dragEnterEvent accepted";
        QList<QUrl> urls = event->mimeData()->urls();
        for (int i = 0; i < urls.size(); ++i) {
            qDebug() << "CustomListView::dragEnterEvent url" << urls.at(i).toString();
        }
    } else {
        qDebug() << "CustomListView::dragEnterEvent rejected";
        event->ignore();
    }
    QListView::dragEnterEvent(event);
}

void CustomListView::dragMoveEvent(QDragMoveEvent* event)
{
    qDebug() << "CustomListView::dragMoveEvent";

    if (event->mimeData()->hasFormat("text/uri-list")) {
        event->acceptProposedAction();
        qDebug() << "CustomListView::dragMoveEvent accepted";
        QList<QUrl> urls = event->mimeData()->urls();
        for (int i = 0; i < urls.size(); ++i) {
            qDebug() << "CustomListView::dragMoveEvent url" << urls.at(i).toString();
        }
    } else {
        qDebug() << "CustomListView::dragMoveEvent rejected";
        event->ignore();
    }

    // Spring-loaded folders
    QModelIndex index = indexAt(event->pos());

    if (index.isValid()) {
        if (!m_springTimer.isActive()) {
            m_potentialTargetIndex = index;
            qDebug() << "CustomListView::dragMoveEvent m_potentialTargetIndex.isValid()";
            m_springTimer.start(1000); // Start the timer with a 1-second delay
        }
    } else {
        m_springTimer.stop();
        m_potentialTargetIndex = QModelIndex();
        qDebug() << "CustomListView::dragMoveEvent !m_potentialTargetIndex.isValid()";
    }

    // Call super class dragMoveEvent
    QListView::dragMoveEvent(event);
}

void CustomListView::dragLeaveEvent(QDragLeaveEvent *event) {
    qDebug() << "CustomListView::dragLeaveEvent";

    // Spring-loaded folders
    m_springTimer.stop();
    m_potentialTargetIndex = QModelIndex();

    QAbstractItemView::dragLeaveEvent(event);
}

void CustomListView::expandTarget() {
    qDebug() << "CustomListView::expandTarget";

    // Spring-loaded folders
    if (m_potentialTargetIndex.isValid()) {
        qDebug() << "CustomListView::expandTarget m_potentialTargetIndex.isValid()";
        QString path = m_potentialTargetIndex.data(QFileSystemModel::FilePathRole).toString();
        // Get the widget in which this view is
        qDebug() << "CustomListView::expandTarget path" << path;
        // Check if it is an application bundle
        ApplicationBundle* app = new ApplicationBundle(path);
        if (app->isValid()) {
            qDebug() << "CustomListView::expandTarget app->isValid()";
            // Launch the app
            // TODO: We should not do this here after the delay, but immediately?
            qDebug() << "TODO: Launch the app";
        } else if (QFileInfo(path).isDir()) {
            qDebug() << "CustomListView::expandTarget !app->isValid()";
            // Open the folder
            qDebug() << "Open the spring-loaded folder";
            FileManagerMainWindow *fileManagerMainWindow = qobject_cast<FileManagerMainWindow *>(QApplication::activeWindow());

            // Close the spring-loaded folder once it is no longer needed; except for the Desktop
            bool parentIsDesktop = false;
            if (QFileInfo(path).dir().path() == QDir::homePath() + "/Desktop") {
                parentIsDesktop = true;
            }
            if (!fileManagerMainWindow->instanceExists(path)) {
                fileManagerMainWindow->openFolderInNewWindow(path);
            } else {
                qDebug() << "Window already open for" << path;
                // Raise the window
                fileManagerMainWindow->getInstanceForDirectory(path)->raise();
            }
            if (!parentIsDesktop) {
                // Check if a window is open for the parent dir of path and if it is, close it
                QString parentPath = QFileInfo(path).dir().path();
                qDebug() << "parentPath" << parentPath;
                if (fileManagerMainWindow->instanceExists(parentPath)) {
                    qDebug() << "Closing spring-loaded folder for" << parentPath;
                    fileManagerMainWindow->getInstanceForDirectory(parentPath)->close();
                }
            }

        } else {
            qDebug() << "Ignoring this item";
        }

        delete app;
    }
    m_springTimer.stop();
}

void CustomListView::dropEvent(QDropEvent* event)
{
    qDebug() << "CustomListView::dropEvent";

    // NOTE: For this to work, the QListView must have viewport()->setAcceptDrops(true) set
    // and the model must have supportedDragActions and supportedDropActions methods

    // Print the MIME types
    QStringList formats = event->mimeData()->formats();
    for (int i = 0; i < formats.size(); ++i) {
        qDebug() << "CustomListView::dropEvent format" << formats.at(i);
    }

    QList<QUrl> urls = event->mimeData()->urls();

    // Get data
    const QMimeData* data = event->mimeData();

    // Check if all dropped urls start with http:// or https://
    bool all_urls_are_http = true;
    for (int i = 0; i < urls.size(); ++i) {
        if (!urls.at(i).toString().startsWith("http://") && !urls.at(i).toString().startsWith("https://")) {
            all_urls_are_http = false;
        }
    }

    // Create a desktop file for URLs dropped from the web browser
    // TODO: Move this into the model? So that all views can use it?
    if (all_urls_are_http) {
        qDebug() << "CustomListView::dropEvent all_urls_are_http";
    }

    // Check if all dropped urls start with file://
    bool all_urls_are_file = true;
    for (int i = 0; i < urls.size(); ++i) {
        if (!urls.at(i).toString().startsWith("file://")) {
            all_urls_are_file = false;
        }
    }

    if (all_urls_are_file) {
        qDebug() << "CustomListView::dropEvent all_urls_are_file";
    }
    QListView::dropEvent(event);

    // Let the model handle the drop event
    // QUESTION: Is this the correct way to do it? Is this documented anywhere?
    QModelIndex index = indexAt(event->pos());
    int row = index.row();
    int column = index.column();
    model()->dropMimeData(event->mimeData(), event->dropAction(), row, column, index);
}

void CustomListView::startDrag(Qt::DropActions supportedActions)
{
    qDebug() << "CustomListView::startDrag";
    QListView::startDrag(supportedActions);
}