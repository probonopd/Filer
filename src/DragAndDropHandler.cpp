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

#include "DragAndDropHandler.h"
#include "CustomListView.h"
#include <QAbstractItemView>
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
#include <QFileInfo>
#include <QMenu>
#include <QDrag>
#include "FileOperationManager.h"
#include <QProcess>
#include <TrashHandler.h>
#include <QUrl>
#include <QDir>
#include <Mountpoints.h>

DragAndDropHandler::DragAndDropHandler(QAbstractItemView *view, QObject *parent)
        : QObject(parent), m_view(view) {


    // NOTE: the model used also has to provide support for drag and drop operations.
    // The actions supported by a model can be specified by reimplementing the
    // QAbstractItemModel::supportedDropActions() function.
    // NOTE: Enabling drag and drop for items
    // Models indicate to views which items can be dragged, and which will accept drops,
    // by reimplementing the QAbstractItemModel::flags() function to provide suitable flags.

    m_view->setAcceptDrops(true);
    m_view->setDragEnabled(true);
    m_view->setDropIndicatorShown(true);
    m_view->setDefaultDropAction(Qt::MoveAction);

    // https://doc.qt.io/qt-5/model-view-programming.html#using-convenience-views
    // Enable the user to move the items around within the view
    m_view->setDragDropMode(QAbstractItemView::InternalMove); // Adjust as needed

    m_view->viewport()->setAcceptDrops(true);

    // Spring-loaded folders
    connect(&m_springTimer, &QTimer::timeout, this, &DragAndDropHandler::mouseHoversOver);
}

void DragAndDropHandler::handleDragEnterEvent(QDragEnterEvent* event)
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
    // m_view->dragEnterEvent(event);
}

void DragAndDropHandler::handleDragMoveEvent(QDragMoveEvent* event)
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
    QModelIndex index = m_view->indexAt(event->pos());

    if (index.isValid()) {
        if (!m_springTimer.isActive()) {
            m_potentialTargetIndex = index;
            qDebug() << "CustomListView::dragMoveEvent m_potentialTargetIndex.isValid()";
            m_springTimer.start(2000); // Start the timer with a 2-second delay
        }
    } else {
        m_springTimer.stop();
        m_potentialTargetIndex = QModelIndex();
        qDebug() << "CustomListView::dragMoveEvent !m_potentialTargetIndex.isValid()";
    }

    // Call super class dragMoveEvent
    // m_view->dragMoveEvent(event);
}

void DragAndDropHandler::handleDragLeaveEvent(QDragLeaveEvent *event) {
    qDebug() << "CustomListView::dragLeaveEvent";

    // Spring-loaded folders
    m_springTimer.stop();
    m_potentialTargetIndex = QModelIndex();

    // m_view->dragLeaveEvent(event);
}

void DragAndDropHandler::mouseHoversOver() {
    qDebug() << "CustomListView::mouseHoversOver";

    // Spring-loaded folders
    if (m_potentialTargetIndex.isValid()) {
        QString path = m_potentialTargetIndex.data(QFileSystemModel::FilePathRole).toString();
        // If the item dragged onto is the ~/Desktop/Trash, don't spring-load it
        if (path == QDir::homePath() + "/Desktop/Trash") {
            qDebug() << "Ignoring spring-loading because it is the Trash";
            return;
        }
        // Check the item dragged onto is an application bundle
        ApplicationBundle* app = new ApplicationBundle(path);
        if (app->isValid()) {
            qDebug() << "Ignoring spring-loading because it is an application bundle";
            // This is handled elsewhere
        } else if (QFileInfo(path).isDir()) {
            qDebug() << "CustomListView::mouseHoversOver !app->isValid()";
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
                fileManagerMainWindow->getInstanceForDirectory(path)->bringToFront();
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

void DragAndDropHandler::handleDropEvent(QDropEvent* event)
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

    // Check if all dropped urls start with file://
    bool all_urls_are_file = true;
    for (int i = 0; i < urls.size(); ++i) {
        if (!urls.at(i).toString().startsWith("file://")) {
            all_urls_are_file = false;
        }
    }

    if (all_urls_are_file) {
        qDebug() << "CustomListView::dropEvent all_urls_are_file";

        // Check if the files are being dropped onto an application bundle; if so, open the files with the app
        // TODO: Move this code somewhere else outside of CustomListView so that it can be used by other views?
        // It would be best to move it to a class that is shared by all views, or possibly
        // to CustomFileSystemModel::canDropMimeData, but that would require the model to be aware of the view
        // (e.g., to know what something is being dropped onto)
        // How to do this correctly?

        QString targetPath = m_potentialTargetIndex.data(QFileSystemModel::FilePathRole).toString();
        if (!targetPath.isEmpty()) {
            qDebug() << "CustomListView::dropEvent targetPath" << targetPath;
            // Check if the target is the Trash
            if (targetPath == QDir::homePath() + "/Desktop/Trash") {
                qDebug() << "CustomListView::dropEvent targetPath is Trash";
                QStringList paths = {};
                for (int i = 0; i < urls.size(); ++i) {
                    paths.append(urls.at(i).toLocalFile());
                }
                TrashHandler *th = new TrashHandler();
                th->moveToTrash({paths});
                delete th;
                return;
            }
            // Check if the target is an application bundle
            ApplicationBundle* app = new ApplicationBundle(targetPath);
            if (app->isValid()) {
                for (int i = 0; i < urls.size(); ++i) {
                    // Convert the url to a local path
                    QString path = urls.at(i).toLocalFile();
                    // Check MIME type of the file
                    QString mimeType = QMimeDatabase().mimeTypeForFile(path).name();
                    qDebug() << "CustomListView::dropEvent mimeType" << mimeType;
                    // TODO: Check if the app supports the MIME type or whether a modifier key is pressed
                    app->launch({path});
                }
                delete app;
                return;
            }
            delete app;
            // Check if the target is a directory
            if (QFileInfo(targetPath).isDir()) {
                qDebug() << "CustomListView::dropEvent targetPath is a directory";
                // Show the drop menu
                showDropMenu(event, urls, targetPath);
                event->acceptProposedAction();
                return;
            }
        }

        // Find out the directory where the files are being dropped in case they are not being dropped onto an item
        if (targetPath.isEmpty()) {
            // If the target path is empty, the files are being dropped
            // onto the root of the view, so use the current directory
            // So get the model for this view and get the current directory
            CustomProxyModel* model = static_cast<CustomProxyModel*>(m_view->model());
            CustomFileSystemModel* sourceModel = static_cast<CustomFileSystemModel*>(model->sourceModel());
            // Get the root index and from it, get the current directory
            QModelIndex rootIndex = sourceModel->index(sourceModel->rootPath());
            targetPath = rootIndex.data(QFileSystemModel::FilePathRole).toString();
        }

        // Check if the destination is the parent directory of the source directory
        // of all the files being dropped; if so, do nothing
        bool parentIsSource = false;
        for (int i = 0; i < urls.size(); ++i) {
            // Convert the url to a local path
            QString path = urls.at(i).toLocalFile();
            // Check if the parent directory of the source directory of the file
            // being dropped is the same as the destination directory
            if (QFileInfo(path).dir().path() == targetPath) {
                parentIsSource = true;
            }
        }
        if (parentIsSource) {
            qDebug() << "CustomListView::dropEvent parentIsSource";
            qDebug() << "CustomListView::dropEvent Ignoring this drop because the parent directory of the source directory is the same as the destination directory";
            return;
        }
        showDropMenu(event, urls, targetPath);

    }

    // Let the model handle the drop event
    // QUESTION: Is this the correct way to do it? Is this documented anywhere?
    QModelIndex index = m_view->indexAt(event->pos());
    int row = index.row();
    int column = index.column();
    m_view->model()->dropMimeData(event->mimeData(), event->dropAction(), row, column, index);
}

void DragAndDropHandler::showDropMenu(QDropEvent *event, QList<QUrl> &urls,
                                      const QString &targetPath) const {// Show a context menu asking what to do with the files
// Copy, Move, Link, Cancel
    QMenu menu(m_view);
    QAction *copyAction = menu.addAction("Copy");
    QAction *moveAction = menu.addAction("Move");
    QAction *linkAction = menu.addAction("Link");
    menu.addSeparator();
    QAction *cancelAction = menu.addAction("Cancel");
    // Show the menu at the global mouse coordinates
    // Get the coordinates of the mouse
    QPoint mousePos = event->pos();
    // Map to global coordinates
    mousePos = m_view->viewport()->mapToGlobal(mousePos);
    QAction *selectedAction = menu.exec(mousePos);
    if (selectedAction == copyAction) {
        qDebug() << "CustomListView::dropEvent copyAction";
        event->setDropAction(Qt::CopyAction);
        QStringList sourceFilePaths;
        for (const QUrl &url : urls) {
            sourceFilePaths.append(url.toLocalFile());
            qDebug() << "Shall copy " << url.toLocalFile() << " to " << targetPath;
        }
        FileOperationManager::copyWithProgress(sourceFilePaths, targetPath);
    } else if (selectedAction == moveAction) {
        qDebug() << "CustomListView::dropEvent moveAction";
        event->setDropAction(Qt::MoveAction);
        QStringList sourceFilePaths;
        for (const QUrl &url : urls) {
            sourceFilePaths.append(url.toLocalFile());
            qDebug() << "Shall move " << url.toLocalFile() << " to " << targetPath;
        }
        FileOperationManager::moveWithProgress(sourceFilePaths, targetPath);
    } else if (selectedAction == linkAction) {
        qDebug() << "CustomListView::dropEvent linkAction";
        event->setDropAction(Qt::LinkAction);
        // Symlink the files
        bool success = true;
        QStringList linkPaths;
        for (int i = 0; i < urls.size(); ++i) {
            // Convert the url to a local path
            QString path = urls.at(i).toLocalFile();
            // Skip source if its parent directory is the same as the destination directory
            if (QFileInfo(path).dir().path() == targetPath) {
                continue;
            }
            // Make a symlink
            QString linkPath = targetPath + "/" + QFileInfo(path).fileName();
            linkPaths.append(linkPath);

            // Check whether the parent directory of the symlink is writable by the user
            // If not, use sudo -A -E ln -s
            bool useSudo = false;
            QString targetDir = QFileInfo(linkPath).dir().path();
            useSudo = ! FileOperationManager::areTreesAccessible({targetDir}, FileOperationManager::Writable);
            if (useSudo == false) {
                qDebug() << "Creating symlink from " << path << " to " << linkPath;
                bool result = QFile::link(path, linkPath);
                if (!result) {
                    qDebug() << "CustomListView::dropEvent Failed to create symlink";
                    success = false;
                } else {
                    qDebug() << "CustomListView::dropEvent Symlink created";

                }
            } else {
                QProcess *process = new QProcess();
                QStringList args;
                args << "-A" << "-E" << "ln" << "-s" << path << linkPath;
                qDebug() << "Creating symlink from " << path << " to " << linkPath << " using sudo";
                process->start("sudo", args);
                process->waitForFinished();
                if (process->exitCode() != 0) {
                    qDebug() << "CustomListView::dropEvent Failed to create symlink";
                    success = false;
                } else {
                    qDebug() << "CustomListView::dropEvent Symlink created";
                }
            }

        }
        if(success) {
            // Show the links by re-using the DBusInterface class which can do this,
            // but without doing an actual DBus call
            DBusInterface *dbi = new DBusInterface();
            QStringList urls;
            for (int i = 0; i < linkPaths.size(); ++i) {
                urls.append(QUrl::fromLocalFile(linkPaths.at(i)).toString());
            }
            dbi->ShowItems(urls, QString("startUpId"));
            delete dbi;
        }
    } else if (selectedAction == cancelAction) {
        qDebug() << "CustomListView::dropEvent cancelAction";
        // TODO: Move the items back to where they came from
        // TODO: How to do this? How to get the original position of the items?
        event->setDropAction(Qt::IgnoreAction);
    }
    qDebug() << "event->dropAction() set to:" << event->dropAction();

    // Accept the event
    event->accept(); // Not sure whether this should be done here or in the model
}

void DragAndDropHandler::handleStartDrag(Qt::DropActions supportedActions) {
    qDebug() << "DragAndDropHandler::startDrag";
    // Get the selected items
    QModelIndexList selectedIndexes = m_view->selectionModel()->selectedIndexes();
    // Get the model for this view
    CustomProxyModel* model = static_cast<CustomProxyModel*>(m_view->model());
    CustomFileSystemModel* sourceModel = static_cast<CustomFileSystemModel*>(model->sourceModel());
    // Get the root index and from it, get the current directory
    QModelIndex rootIndex = sourceModel->index(sourceModel->rootPath());
    QString currentDirectory = rootIndex.data(QFileSystemModel::FilePathRole).toString();
    // Get the list of paths for the selected items
    QStringList paths;
    for (int i = 0; i < selectedIndexes.size(); ++i) {
        QModelIndex index = selectedIndexes.at(i);
        // Get the url for this index
        QString path = index.data(QFileSystemModel::FilePathRole).toString();
        // Add it to the list of urls
        paths.append(path);
    }
    // Create a mime data object
    QMimeData *mimeData = new QMimeData;
    // Convert the paths to a list of QUrls
    QList<QUrl> urlList;
    for (int i = 0; i < paths.size(); ++i) {
        urlList.append(QUrl::fromLocalFile(paths.at(i)));
    }
    // Set the urls as the mime data
    mimeData->setUrls(urlList);
    // Create a drag object
    QDrag *drag = new QDrag(m_view);
    // Set the mime data for the drag object
    drag->setMimeData(mimeData);

    // Set the icon for the drag object so that while dragging, the icon being dragged is actually shown
    // Get the icon for the first item
    QModelIndex firstIndex = selectedIndexes.at(0);
    QIcon icon = model->data(firstIndex, Qt::DecorationRole).value<QIcon>();
    // Get the pixmap for the icon
    // TODO: Which pixmap to use if there are multiple items?
    // The one for the first item? Or the one for the whole selection?
    // Or make a special "multiple items" icon?
    QPixmap pixmap = icon.pixmap(64, 64);
    // Set the pixmap for the drag object
    drag->setPixmap(pixmap);
    // Set coordinates for the drag object
    drag->setHotSpot(QPoint(pixmap.width()/2, pixmap.height()/2));

    // Start the drag
    drag->exec(supportedActions);

}