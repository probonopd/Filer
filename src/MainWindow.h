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

/*
 * This code defines a class called FileManagerMainWindow, which is derived from the QMainWindow
 * class. It provides a GUI for a file manager application that allows the user to browse and manage
 * files and directories. The class has several private member variables, including m_treeView and
 * m_iconView, which are pointers to QTreeView and QListView objects, respectively. These objects
 * will be used to display the file system hierarchy in a tree view and a list view. The class also
 * has several private member functions, such as showTreeView() and showIconView(), which are used
 * to switch between the tree view and the list view.
 */

#ifndef FILEMANAGERMAINWINDOW_H
#define FILEMANAGERMAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QTreeView>
#include <QListView>
#include <QStatusBar>
#include <QList>
#include <QStackedWidget>
#include "CustomFileSystemModel.h"

class FileManagerMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static QList<FileManagerMainWindow *> &
    instances(); // The list is declared static, so it is unique for the entire application

    FileManagerMainWindow(QWidget *parent = nullptr, const QString &initialDirectory = "/");

    ~FileManagerMainWindow();

    // Getter and setter methods for the directory property
    QString directory() const;
    void setDirectory(const QString &directory);

    void showAboutBox();

    void showTreeView();
    void showIconView();
    void showHideStatusBar();

    // QFileSystemModel *m_fileSystemModel;
    CustomFileSystemModel *m_fileSystemModel;

    bool m_is_first_instance = false;

public slots:
    void open(const QString &filePath);
    void openWith(const QString &filePath);
    void openFolderInNewWindow(const QString &rootPath);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void renameSelectedItem();
    void resizeEvent(QResizeEvent *event);

    void moveEvent(QMoveEvent *event);
    void
    paintEvent(QPaintEvent *event); // Overriding the paintEvent() method to draw a background image

    void dropEvent(QDropEvent *event);

private:
    QStackedWidget *m_stackedWidget;

    QString m_currentDir;
    QTreeView *m_treeView;
    QListView *m_iconView;
    QItemSelectionModel *m_selectionModel;
    QMenuBar *m_menuBar;

    QStatusBar *m_statusBar;
    QAction *m_treeViewAction;
    QAction *m_iconViewAction;

    QAction *m_renameAction;

    QStringList readFilenamesFromHiddenFile(const QString &filePath);
    void openFolder(const QString &rootPath);

    void createMenus();

    void selectAll();
    void updateStatusBar();
    void copySelectedItems(const QString &destinationPath);
    void moveSelectedItems(const QString &destinationPath);
    void onItemDropped(const QModelIndex &targetIndex);
    void enableDropOnFolder(const QModelIndex &index);
    void updateMenus();

    void saveWindowGeometry();
};

#endif // FILEMANAGERMAINWINDOW_H
