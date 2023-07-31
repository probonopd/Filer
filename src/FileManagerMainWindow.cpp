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

#include "FileManagerMainWindow.h"
#include "CustomItemDelegate.h"
#include "CustomFileIconProvider.h"
#include "ExtendedAttributes.h"

#include <QMenuBar>
#include <QMenu>
#include <QMainWindow>
#include <QTreeView>
#include <QAction>
#include <QMessageBox>
#include <QDebug>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QProcess>
#include <QInputDialog>
#include <QApplication>
#include <QGuiApplication>
#include <QWindow>
#include <QTimer>
#include <QScreen>
#include <Qt>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QStandardPaths>
#include <QShortcut>
#include <QKeySequence>
#include <QPainter>
#include <QPen>
#include <QRect>
#include <QLinearGradient>
#include <QColor>
#include <QSortFilterProxyModel>
#include <QMimeData>
#include <QThread>
#include <QLabel>
#include <QDialogButtonBox>
#include <QFileSystemModel>
#include <QCompleter>
#include <QRegExpValidator>
#include "ApplicationBundle.h"
#include "TrashHandler.h"

/*
 * This creates a FileManagerMainWindow object with a QTreeView and QListView widget.
 * The QTreeView widget displays the file system hierarchy, and the QListView widget displays
 * the files and directories in the selected directory. The QFileSystemModel class is used
 * to provide data for the views. The QItemSelectionModel class is used to manage the selection
 * of items in the views. The QMenuBar and QMenu classes are used to create a menu bar with a menu.
 * The menu has actions for switching between the tree and icon views.
 * The QStatusBar class is used to create a status bar that shows the number of items selected in
 * the views.
 */

QList<FileManagerMainWindow *> &FileManagerMainWindow::instances()
{
    static QList<FileManagerMainWindow *> instances;
    return instances;
}

FileManagerMainWindow::FileManagerMainWindow(QWidget *parent, const QString &initialDirectory)
    : QMainWindow(parent)
{
    qDebug() << "FileManagerMainWindow::FileManagerMainWindow()";

    m_currentDir = initialDirectory;

    ExtendedAttributes extendedAttributes(m_currentDir);

    // Get number of instances
    int instanceCount = instances().count();
    qDebug() << "instanceCount:" << instanceCount;

    if (instanceCount == 0) {
        m_isFirstInstance = true;
        // If this is the first window, set this window as the root window of the main screen
        // Check if ~/Desktop exists; create it if it doesn't
        QDir homeDir(QDir::homePath());
        if (!homeDir.exists("Desktop")) {
            homeDir.mkdir("Desktop");
        }

        // Set the object name to "Desktop" so that we can find it later
        setObjectName("Desktop");

        // Set the root path to ~/Desktop
        setDirectory(QDir::homePath() + "/Desktop");

        qDebug() << "First instance, show the desktop";
        setFixedSize(QApplication::desktop()->screenGeometry(0).size());
        setAttribute(Qt::WA_X11NetWmWindowTypeDesktop, true);

    } else {
        m_isFirstInstance = false;
        setDirectory(initialDirectory);

        // Read extended attributes describing the window geometry
        qDebug() << "Reading extended attributes";

        QByteArray positionAndGeometry = extendedAttributes.read("positionAndGeometry");
        // from qbytearray to qstring
        QString positionAndGeometryString = QString::fromUtf8(positionAndGeometry);
        qDebug() << "positionAndGeometryString:" << positionAndGeometryString;

        // Check if contains only digits and 3 "," in between
        QRegExp rx("^[0-9]+,[0-9]+,[0-9]+,[0-9]+$");
        if (rx.exactMatch(positionAndGeometryString)) {
            qDebug() << "positionAndGeometryString is valid";
            // from qstring to qrect
            QStringList positionAndGeometryList = positionAndGeometryString.split(",");
            QRect positionAndGeometryRect =
                    QRect(positionAndGeometryList[0].toInt(), positionAndGeometryList[1].toInt(),
                          positionAndGeometryList[2].toInt(), positionAndGeometryList[3].toInt());
            qDebug() << "positionAndGeometryRect:" << positionAndGeometryRect;
            setGeometry(positionAndGeometryRect);
        } else {
            qDebug() << "positionAndGeometryString is invalid";
            resize(600, 400);
            // move(100, 100);
        }
        // If the window is outside the screen, move it to the center of the screen
        if (!QApplication::desktop()->screenGeometry().contains(geometry())) {
            qDebug() << "Window is outside the screen";
            move(QApplication::desktop()->screen()->rect().center() - rect().center());
        }
    }

    // Append to the list of windows
    instances().append(this);

    // Set type of window to be a file manager window
    setProperty("type", "filemanager");

    // Set the icon for the window
    setWindowIcon(QIcon::fromTheme("folder"));

    // Initialize m_stackedWidget
    m_stackedWidget = new QStackedWidget(this);

    // Create the tree view and list view
    m_treeView = new QTreeView(this);
    m_iconView = new CustomListView(this);

    // Add the tree view and list view to the stacked widget
    m_stackedWidget->addWidget(m_treeView);
    m_stackedWidget->addWidget(m_iconView);

    // Set the stacked widget as the central widget
    setCentralWidget(m_stackedWidget);

    // Make the tree view sortable
    m_treeView->setSortingEnabled(true);

    // No frame around the views
    m_treeView->setFrameStyle(QFrame::NoFrame);
    m_iconView->setFrameStyle(QFrame::NoFrame);

    // Draw the desktop picture for the first instance
    if (m_isFirstInstance) {
        m_iconView->requestDesktopPictureToBePainted(true);
    }

    // Create an instance of our custom QFileIconProvider
    CustomFileIconProvider provider;

    // m_fileSystemModel = new QFileSystemModel(this);
    m_fileSystemModel = new CustomFileSystemModel(this); // Not really working correctly yet; FIXME

    provider.setModel(m_fileSystemModel);

    // Make the file system model use the custom icon provider
    m_fileSystemModel->setIconProvider(&provider);

    // Filter out items that should not be shown
    m_fileSystemModel->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot); // | QDir::Hidden

    // Make it use the icon provider on files as well
    m_fileSystemModel->setResolveSymlinks(true);

    //////////////////////////////////
    // TODO: Hide hidden files
    //////////////////////////////////

    // Set the file system model as the model for the tree view and icon view
    m_treeView->setModel(m_fileSystemModel);
    m_iconView->setModel(m_fileSystemModel);

    // Set the root path to the specified m_currentDir
    m_fileSystemModel->setRootPath(m_currentDir);

    // Preload the data for the tree view (so that e.g., CustomIconProvider can know open-with attributes)
    m_treeView->setRootIndex(m_fileSystemModel->index(m_currentDir));
    m_iconView->setRootIndex(m_fileSystemModel->index(m_currentDir));


    //////////////////////////////////////////////////////
    // TODO: Add entries to the Desktop for Trash, Volumes
    // by adding items to the file system model
    m_fileSystemModel->insertRow(0, m_fileSystemModel->index(m_currentDir));
    m_fileSystemModel->setData(
            m_fileSystemModel->index(0, 0, m_fileSystemModel->index(m_currentDir)), "Trash");
    //////////////////////////////////////////////////////

    // Set the window title to the root path of the QFileSystemModel
    setWindowTitle(QFileInfo(m_fileSystemModel->rootPath()).fileName());

    // Create an instance of the CustomItemDelegate class;
    // we need this so that we have control over how the items (icons with text)
    // get drawn

    CustomItemDelegate *customItemDelegate = new CustomItemDelegate(m_stackedWidget->currentWidget(), m_fileSystemModel);

    // Install the custom item delegate as an event filter on the tree view and icon view
    // so that we can intercept mouse events like QEvent::DragMove
    m_treeView->viewport()->installEventFilter(customItemDelegate);
    m_iconView->viewport()->installEventFilter(customItemDelegate);

    m_iconView->setItemDelegate(customItemDelegate);
    m_treeView->setItemDelegate(customItemDelegate);

    // Create the selection model for the tree view and icon view
    m_selectionModel = new QItemSelectionModel(m_fileSystemModel);

    // Set the selection model for the tree view and icon view
    m_treeView->setSelectionModel(m_selectionModel);
    m_iconView->setSelectionModel(m_selectionModel);

    customItemDelegate->setSelectionModel(m_selectionModel); // Set the selection model

    // Create the menu bar
    m_menuBar = new QMenuBar(this);

    // Set the menu bar
    setMenuBar(m_menuBar);

    // Set the background color of the menu bar
    // to the background color of QWidgets
    m_menuBar->setStyleSheet("QMenuBar { background-color: palette(window); }");

    // Use full width for the menu bar
    m_menuBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // Initialize the Tree View and Icon View actions
    m_iconViewAction = new QAction("Icon View", this);
    m_iconViewAction->setCheckable(true);
    m_iconViewAction->setChecked(true);
    m_treeViewAction = new QAction("Tree View", this);
    m_treeViewAction->setCheckable(true);
    m_treeViewAction->setChecked(false);

    // Create the menus
    createMenus();

    // Create the status bar
    m_statusBar = new QStatusBar(this);

    // Set the status bar
    setStatusBar(m_statusBar);
    m_statusBar->hide();

    // Set the width of the first column
    m_treeView->setColumnWidth(0, 400);

    // Set the selection mode to ExtendedSelection
    m_treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_iconView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Set the elision mode to elide ("...") the text in the middle
    m_treeView->setTextElideMode(Qt::ElideMiddle);
    m_iconView->setTextElideMode(Qt::ElideMiddle);

    // To call the updateStatusBar() function whenever the selected items change,
    // Connect the selectionChanged() signal of the selection model to the updateStatusBar() slot
    connect(m_selectionModel, &QItemSelectionModel::selectionChanged, this,
            &FileManagerMainWindow::updateStatusBar);
    updateStatusBar();

    // To call the updateMenus() function whenever the selected items change,
    // connect the QItemSelectionModel's selectionChanged() signal to the updateMenus() slot
    connect(m_selectionModel, &QItemSelectionModel::selectionChanged, this,
            &FileManagerMainWindow::updateMenus);
    updateMenus();

    // Connect the doubleClicked() signal to the open() slot
    connect(
            m_iconView, &QTreeView::doubleClicked, this,
            [this](const QModelIndex &index) {
                QString filePath = m_fileSystemModel->filePath(index);
                open(filePath);
            },
            Qt::QueuedConnection);

    /* Tree view */

    // Set the default item size
    m_treeView->setIndentation(10);

    // Show the columns for file size, file type, and last modified date
    m_treeView->setHeaderHidden(false);

    // Set alternating colors for the items in the tree view
    m_treeView->setAlternatingRowColors(true);

    /* Icon view */

    // If this is the first window, arrange icons differently
    if (instanceCount == 0) {
        // Set the flow property to TopToBottom
        m_iconView->setFlow(QListView::TopToBottom);
        // Mirror the layout of the icons
        m_iconView->setLayoutDirection(Qt::RightToLeft);
        // Make the items the same size to make the layout look orderly
        m_iconView->setUniformItemSizes(true);
    }

    // Set the icon size to 32x32 pixels
    m_iconView->setIconSize(QSize(32, 32));
    m_treeView->setIconSize(QSize(16, 16));

    // Set the view mode to IconMode with the text under the icons and make the icons freely movable
    m_iconView->setViewMode(QListView::IconMode);
    m_iconView->setMovement(QListView::Free);

    // Put the icons on a grid
    QSize iconSize = m_iconView->iconSize();
    m_iconView->setGridSize(QSize(200, 64));

    // Connect the doubleClicked() signal to the open() slot
    connect(
            m_treeView, &QTreeView::doubleClicked, this,
            [this](const QModelIndex &index) {
                QString filePath = m_fileSystemModel->filePath(index);
                open(filePath);
            },
            Qt::QueuedConnection);

    /* Overall */

    // showTreeView();
    // showIconView();

    if (instanceCount != 0) {
        // Read extended attribute describing the view mode
        QByteArray viewMode = extendedAttributes.read("WindowView");
        int viewModeInt = viewMode.toInt();
        qDebug() << "viewModeString:" << viewModeInt;
        if (viewModeInt == 1) {
            // Set the central widget to the list view (icons)
            showTreeView();
        } else {
            // Set the central widget to the tree view
            showIconView();
        }
    } else {
        // The desktop is always shown as icons
        showIconView();
    }

    // If this is the first instance, disable m_showHideStatusBarAction
    if (instanceCount == 0) {
        m_showHideStatusBarAction->setEnabled(false);
    }

    // Call destructor and destroy the window immediately when the window is closed
    // Only this way the window will be destroyed immediately and not when the event loop is
    // finished and we can remove the window from the list of child windows of the parent window
    setAttribute(Qt::WA_DeleteOnClose);
}

// Saves the window geometry
void FileManagerMainWindow::saveWindowGeometry()
{
    // Print window positionAndGeometry
    qDebug() << "Window positionAndGeometry: " << geometry();
    qDebug() << "Window size: " << size();

    // Get the number of the display the window is on
    int displayNumber = QGuiApplication::screens().indexOf(windowHandle()->screen());
    qDebug() << "Window display number: " << displayNumber;

    // Get the filename of the current directory
    QString currentDir =
            m_fileSystemModel->filePath(m_fileSystemModel->index(m_fileSystemModel->rootPath()));

    // Write extended attribute to the current directory
    ExtendedAttributes ea = ExtendedAttributes(currentDir);

    // Writing the window position and geometry directly as a QByteArray does not work because it
    // contains null bytes, so we convert it to a string
    QString positionAndGeometry = QString::number(geometry().x()) + "," + QString::number(geometry().y())
            + "," + QString::number(geometry().width()) + ","
            + QString::number(geometry().height());
    QByteArray positionAndGeometryByteArray = positionAndGeometry.toUtf8();

    // Write the window positionAndGeometry to an extended attribute
    ea.write("positionAndGeometry", positionAndGeometryByteArray);

    // If "Tree View" is checked in the "View" menu, write "1" to the extended attribute,
    // otherwise write "2" to the extended attribute
    if (m_treeViewAction->isChecked()) {
        ea.write("WindowView", "1");
    } else {
        ea.write("WindowView", "2");
    }
}

// Callback function for when the user moves the window
void FileManagerMainWindow::moveEvent(QMoveEvent *event)
{
    qDebug() << "moveEvent";

    // Call the base class implementation
    QMainWindow::moveEvent(event);

    // Save the window geometry
    saveWindowGeometry();
}

// Callback function for when the user resizes the window
void FileManagerMainWindow::resizeEvent(QResizeEvent *event)
{
    qDebug() << "resizeEvent";

    // Call the base class implementation
    QMainWindow::resizeEvent(event);

    // Re-layout the items in the view to reflect the new grid layout
    if (!m_treeViewAction->isChecked()) {
        m_iconView->doItemsLayout();
    }

    // TOOD: Wait until no resize events are coming in for 1 second
    // This is necessary because the resizeEvent() is called multiple times when the user resizes
    // the window and we only want to save the window geometry when the user has finished resizing
    // the window. How to do this properly?

    saveWindowGeometry();
}

void FileManagerMainWindow::refresh() {
    qDebug() << "Calling update() on the views";
    m_treeView->update();
    m_iconView->update();
}

FileManagerMainWindow::~FileManagerMainWindow()
{
    qDebug() << "Destructor called";

    // Save the window geometry
    saveWindowGeometry();

    // Remove from the list of windows
    instances().removeAll(this);

    // If this is the last window, quit the application
    if (instances().isEmpty()) {
        qDebug() << "Last window closed, quitting application";
        qApp->quit();
    } else {
        // Check if only one window is left
        if (instances().size() == 1) {
            // Activate the first window so that the focus
            // does not go to another application but to the desktop
            // FIXME: Sometimes the menu bar is mixed up with the menu bar of the other application
            // that was previously active
            qDebug() << "Activating first window";
            instances().first()->activateWindow();
        }
    }

    // Tell all windows that they should be redrawn
    // so that they can update their icons to reflect the new state of
    // folders being open
    QTimer::singleShot(100, []() {
        qDebug() << "Redrawing all windows";
        for (FileManagerMainWindow *window : FileManagerMainWindow::instances()) {
            qDebug("Asking window %s to refresh", window->directory().toUtf8().constData());
            window->refresh();
        }
    });
}

void FileManagerMainWindow::createMenus()
{
    // Remove any previous menus
    menuBar()->clear();

    // Create the File menu
    QMenu *fileMenu = new QMenu(tr("File"));

    // Add the usual menu items to the File menu
    fileMenu->addAction(tr("New..."));
    fileMenu->actions().last()->setShortcut(QKeySequence("Ctrl+N"));
    fileMenu->actions().last()->setEnabled(false);

    // Open
    fileMenu->addAction(tr("Open"));
    m_openAction = fileMenu->actions().last();
    fileMenu->actions().last()->setShortcut(QKeySequence("Ctrl+O"));
    connect(fileMenu->actions().last(), &QAction::triggered, this, [this]() {
        QModelIndex index = m_treeView->currentIndex();
        QString filePath = m_fileSystemModel->filePath(index);
        open(filePath);
    });

    // Open With...
    fileMenu->addAction(tr("Open With..."));
    m_openWithAction = fileMenu->actions().last();
    fileMenu->actions().last()->setShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_O));
    connect(fileMenu->actions().last(), &QAction::triggered, this, [this]() {
        QModelIndex index = m_treeView->currentIndex();
        QString filePath = m_fileSystemModel->filePath(index);
        openWith(filePath);
    });

    // Open and close current
    fileMenu->addAction(tr("Open and close current"));
    fileMenu->actions().last()->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_O));
    connect(fileMenu->actions().last(), &QAction::triggered, [=]() {
        QModelIndex index = m_treeView->currentIndex();
        QString filePath = m_fileSystemModel->filePath(index);
        open(filePath);
        close();
    });
    if (m_isFirstInstance) {
        fileMenu->actions().last()->setEnabled(false);
    }

    fileMenu->addSeparator();

    QAction *closeAction = new QAction(tr("Close"), this);
    closeAction->setShortcut(QKeySequence("Ctrl+W"));
    fileMenu->addAction(closeAction);
    connect(closeAction, &QAction::triggered, this, &FileManagerMainWindow::close);
    if (m_isFirstInstance) {
        closeAction->setEnabled(false);
    }

    fileMenu->addSeparator();

    // Add the File menu to the menu bar
    m_menuBar->addMenu(fileMenu);

    // Create the Edit menu
    QMenu *editMenu = new QMenu(tr("Edit"));

    // Add the usual menu items to the Edit menu
    editMenu->addAction(tr("Undo"));
    editMenu->actions().last()->setShortcut(QKeySequence("Ctrl+Z"));
    // Set disabled because we don't have an undo stack
    editMenu->actions().last()->setEnabled(false);
    editMenu->addSeparator();
    editMenu->addAction(tr("Cut"));
    editMenu->actions().last()->setShortcut(QKeySequence("Ctrl+X"));
    editMenu->addAction(tr("Copy"));
    editMenu->actions().last()->setShortcut(QKeySequence("Ctrl+C"));
    editMenu->addAction(tr("Paste"));
    editMenu->actions().last()->setShortcut(QKeySequence("Ctrl+V"));
    editMenu->addAction(tr("Move to Trash"));
    editMenu->actions().last()->setShortcut(QKeySequence("Ctrl-Backspace"));
    connect(editMenu->actions().last(), &QAction::triggered, this, [this]() {
        // Get all selected indexes
        QModelIndexList selectedIndexes = m_treeView->selectionModel()->selectedIndexes();
        // Get the file paths of the selected indexes
        QStringList filePaths;
                for (const QModelIndex &index : selectedIndexes) {
            filePaths.append(m_fileSystemModel->filePath(index));
        }
        qDebug() << "Moving to trash the following files:";
        for (const QString &filePath : filePaths) {
            qDebug() << filePath;
        }
        // Move the files to the trash
        TrashHandler trashHandler;
        for (const QString &filePath : filePaths) {
            trashHandler.moveToTrash(filePath);
        }
    });
    m_moveToTrashAction = editMenu->actions().last();
    editMenu->addSeparator();
    QAction *selectAllAction = new QAction(tr("Select All"), this);
    selectAllAction->setShortcut(QKeySequence("Ctrl+A"));
    editMenu->addAction(selectAllAction);

    // Connect the triggered() signal of the Select All action to a slot
    connect(selectAllAction, &QAction::triggered, this, &FileManagerMainWindow::selectAll);

    // Create the Rename action
    m_renameAction =
            editMenu->addAction(tr("Rename..."), this, &FileManagerMainWindow::renameSelectedItem);

    // Set the shortcut key for the Rename action to Ctrl+R
    m_renameAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));

    editMenu->addAction(m_renameAction);

    // Add the Edit menu to the menu bar
    m_menuBar->addMenu(editMenu);

    // Create the View menu
    QMenu *viewMenu = new QMenu(tr("View"));

    // Connect the triggered() signals of the Tree View and Icon View actions to slots
    connect(m_treeViewAction, &QAction::triggered, this, &FileManagerMainWindow::showTreeView);
    connect(m_iconViewAction, &QAction::triggered, this, &FileManagerMainWindow::showIconView);

    // Add the Tree View and Icon View actions to the View menu
    viewMenu->addAction(m_treeViewAction);
    if (m_isFirstInstance)
        viewMenu->actions().last()->setEnabled(false);
    viewMenu->addAction(m_iconViewAction);
    if (m_isFirstInstance)
        viewMenu->actions().last()->setEnabled(false);

    viewMenu->addSeparator();

    // Create the Show/Hide Status Bar action
    QAction *showHideStatusBarAction = new QAction(tr("Show/Hide Status Bar"), this);
    m_showHideStatusBarAction = showHideStatusBarAction;

    // Add the Show/Hide Status Bar action to the View menu
    viewMenu->addAction(showHideStatusBarAction);

    // Connect the triggered() signal of the Show/Hide Status Bar action to a slot
    connect(showHideStatusBarAction, &QAction::triggered, this,
            &FileManagerMainWindow::showHideStatusBar);

    // Add the View menu to the menu bar
    m_menuBar->addMenu(viewMenu);

    // Create the Go menu
    QMenu *goMenu = new QMenu(tr("Go"));

    goMenu->addAction(tr("Go Up"));
    goMenu->actions().last()->setShortcut(QKeySequence("Ctrl+Up"));
    if (QFileInfo(m_currentDir).canonicalFilePath() == "/") {
        goMenu->actions().last()->setEnabled(false);
    }
    connect(goMenu->actions().last(), &QAction::triggered, this, [this]() {
        openFolderInNewWindow(QFileInfo(m_currentDir + "/../").canonicalFilePath());
    });
    if (m_isFirstInstance)
        goMenu->actions().last()->setEnabled(false);

    goMenu->addAction(tr("Go Up and Close Current"));
    goMenu->actions().last()->setShortcut(QKeySequence("Shift+Ctrl+Up"));
    if (QFileInfo(m_currentDir).canonicalFilePath() == "/") {
        goMenu->actions().last()->setEnabled(false);
    }
    connect(goMenu->actions().last(), &QAction::triggered, this, [this]() {
        openFolderInNewWindow(QFileInfo(m_currentDir + "/../").canonicalFilePath());
        close();
    });
    if (m_isFirstInstance)
        goMenu->actions().last()->setEnabled(false);

    goMenu->addSeparator();

    goMenu->addAction(tr("Computer"));
    goMenu->actions().last()->setShortcut(QKeySequence("Ctrl+Shift+C"));
    connect(goMenu->actions().last(), &QAction::triggered, this,
            [this]() { openFolderInNewWindow("/"); });

    goMenu->addAction(tr("Network"));
    goMenu->actions().last()->setShortcut(QKeySequence("Ctrl+Shift+N"));
    connect(goMenu->actions().last(), &QAction::triggered, this, [this]() {
        QMessageBox::information(nullptr, (" "), tr("This feature is still in development."));
    });

    goMenu->addAction(tr("Devices"));
    goMenu->actions().last()->setShortcut(QKeySequence("Ctrl+Shift+U"));
    connect(goMenu->actions().last(), &QAction::triggered, this,
            [this]() { openFolderInNewWindow("/media"); });
    goMenu->addAction(tr("Applications"));
    goMenu->actions().last()->setShortcut(QKeySequence("Ctrl+Shift+A"));
    connect(goMenu->actions().last(), &QAction::triggered, this,
            [this]() {
                if (QFileInfo("/Applications").exists()) {
                    openFolderInNewWindow("/Applications");
                } else if (QFileInfo("/usr/local/share/applications").exists()) {
                    openFolderInNewWindow("/usr/local/share/applications");
                } else if (QFileInfo("/usr/share/applications").exists()) {
                    openFolderInNewWindow("/usr/share/applications");
                } else {
                    QMessageBox::information(nullptr, (" "), tr("No applications folder found."));
                }
            });

    goMenu->addSeparator();

    // Create a data structure to hold the information about the menu items
    struct MenuItem {
        QString name;
        QString shortcut;
        QStandardPaths::StandardLocation location;
    };

    // Define the menu items
    const QList<MenuItem> menuItems = {
        {tr("Home"), "Ctrl+Shift+H", QStandardPaths::HomeLocation},
        {tr("Documents"), "Ctrl+Shift+D", QStandardPaths::DocumentsLocation},
        {tr("Downloads"), "Ctrl+Shift+L", QStandardPaths::DownloadLocation},
        {tr("Music"), "Ctrl+Shift+M", QStandardPaths::MusicLocation},
        {tr("Pictures"), "Ctrl+Shift+P", QStandardPaths::PicturesLocation},
        {tr("Videos"), "Ctrl+Shift+V", QStandardPaths::MoviesLocation},
        {tr("Temporary"), "Ctrl+Shift+E", QStandardPaths::TempLocation}
    };

    // Loop through the menu items and create corresponding actions and connections
    for (const MenuItem& menuItem : menuItems) {
        QAction* action = goMenu->addAction(tr(menuItem.name.toUtf8()));
        action->setShortcut(QKeySequence(menuItem.shortcut));
        connect(action, &QAction::triggered, this, [this, location = menuItem.location]() {
            // Create if it doesn't exist
            QDir dir(QStandardPaths::writableLocation(location));
            if (!dir.exists())
                dir.mkpath(".");
            openFolderInNewWindow(QStandardPaths::writableLocation(location));
        });
    }

    goMenu->addSeparator();

    goMenu->addAction(tr("Trash"));
    goMenu->actions().last()->setShortcut(QKeySequence("Ctrl+Shift+T"));
    connect(goMenu->actions().last(), &QAction::triggered, this, [this]() {
        TrashHandler trashHandler;
        QString trashPath = trashHandler.getTrashPath();
        if (!QFileInfo(trashPath).exists()) {
            // Create if it doesn't exist
            QDir dir(trashPath);
            dir.mkpath(".");
            if (!dir.exists()) {
                QMessageBox::information(nullptr, (" "), tr("Could not find the Trash."));
                return;
            }
        }
        openFolderInNewWindow(trashPath);
    });

    goMenu->addSeparator();

    goMenu->addAction(tr("Go to Folder..."));
    goMenu->actions().last()->setShortcut(QKeySequence("Ctrl+Shift+G"));
    connect(goMenu->actions().last(), &QAction::triggered, this, [this]() {
        bool ok;

        QDialog dialog(this);
        dialog.setWindowTitle(tr("Go to Folder..."));
        dialog.setWindowIcon(QIcon::fromTheme("folder"));
        dialog.setFixedWidth(400);
        QVBoxLayout *layout = new QVBoxLayout(&dialog);
        QLabel *label = new QLabel(tr("Folder:"), &dialog);
        layout->addWidget(label);
        QLineEdit *lineEdit = new QLineEdit(&dialog);
        auto completer = new QCompleter(this);
        completer->setCompletionMode(QCompleter::InlineCompletion);
        QFileSystemModel *fsModel = new QFileSystemModel(completer);
        fsModel->setFilter(QDir::Dirs|QDir::Drives|QDir::NoDotAndDotDot|QDir::AllDirs); // Only directories, no files
        completer->setModel(fsModel);
        fsModel->setRootPath(QString());
        lineEdit->setCompleter(completer);
        lineEdit->setPlaceholderText(tr("Enter a folder path..."));
        lineEdit->setClearButtonEnabled(true);
        layout->addWidget(lineEdit);
        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        layout->addWidget(buttonBox);
        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        if (dialog.exec() == QDialog::Accepted) {
            QString text = lineEdit->text();
            QDir dir(text);
            if (dir.exists()) {
                openFolderInNewWindow(text);
            } else {
                QMessageBox::warning(this, tr("Go to Folder..."), tr("The folder does not exist."));
            }
        }
    });

    // Add the Go menu to the menu bar
    m_menuBar->addMenu(goMenu);

    // Create the Help menu
    QMenu *helpMenu = new QMenu("Help");

    // Add an action to the Help menu
    QAction *a = helpMenu->addAction("About Filer");

    // Add the Help menu to the menu bar
    m_menuBar->addMenu(helpMenu);

    // Connect the triggered() signal of the About menu action to a slot
    connect(a, &QAction::triggered, this, &FileManagerMainWindow::showAboutBox);

    // Set up additional shortcuts

    QShortcut *shortcut;

    shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Down), this);
    connect(
            shortcut, &QShortcut::activated, this,
            [this]() {
                QModelIndex index = m_treeView->currentIndex();
                QString filePath = m_fileSystemModel->filePath(index);
                open(filePath);
            },
            Qt::QueuedConnection);

    shortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::CTRL + Qt::Key_Down), this);
    connect(
            shortcut, &QShortcut::activated, this,
            [this]() {
                QModelIndex index = m_treeView->currentIndex();
                QString filePath = m_fileSystemModel->filePath(index);
                openWith(filePath);
            },
            Qt::QueuedConnection);

    shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Down), this);
    connect(
            shortcut, &QShortcut::activated, this,
            [this]() {
                QModelIndex index = m_treeView->currentIndex();
                QString filePath = m_fileSystemModel->filePath(index);
                open(filePath);
                if (!m_isFirstInstance)
                    close();
            },
            Qt::QueuedConnection);

    /*
     * TODO
    shortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    connect(shortcut, &QShortcut::activated, this, &MainWindow::on_actionDelete_triggered);

    shortcut = new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Delete), this);
    connect(shortcut, &QShortcut::activated, this,
    &MainWindow::on_actionDeleteWithoutTrash_triggered);

    shortcut = new QShortcut(QKeySequence(Qt::SHIFT + Qt::CTRL + Qt::Key_Backspace), this);
    connect(shortcut, &QShortcut::activated, this,
    &MainWindow::on_actionDeleteWithoutTrash_triggered);
     */
}

void FileManagerMainWindow::showAboutBox()
{
    // Create the About box
    QMessageBox aboutBox;

    // Set the title and text of the About box
    aboutBox.setWindowTitle("About Filer");
    aboutBox.setText("The helloSystem desktop experience.");
    aboutBox.setIcon(QMessageBox::Information);

    // Show the About box
    aboutBox.exec();
}

void FileManagerMainWindow::selectAll()
{
    m_treeView->selectAll();
    m_iconView->selectAll();   
}

// Show the tree view
void FileManagerMainWindow::showTreeView()
{
    // Print the name of the called function
    qDebug() << Q_FUNC_INFO;

    // Set the checkmark of the Tree View action
    m_treeViewAction->setChecked(true);
    m_iconViewAction->setChecked(false);

    m_stackedWidget->setCurrentWidget(m_treeView);

    // Print a message indicating that the function has completed
    qDebug() << "Completed" << Q_FUNC_INFO;
}

// Show the icon view
void FileManagerMainWindow::showIconView()
{
    // Print the name of the called function
    qDebug() << Q_FUNC_INFO;

    // Set the checkmark of the Icon View action
    m_iconViewAction->setChecked(true);
    m_treeViewAction->setChecked(false);

    m_stackedWidget->setCurrentWidget(m_iconView);

    // Relayout the icon view
    m_iconView->doItemsLayout();

    // Print a message indicating that the function has completed
    qDebug() << "Completed" << Q_FUNC_INFO;
}

// Show or hide the status bar
void FileManagerMainWindow::showHideStatusBar()
{
    // Print the name of the called function
    qDebug() << Q_FUNC_INFO;

    // Check if the status bar is currently visible
    if (m_statusBar->isVisible()) {
        // Hide the status bar
        m_statusBar->hide();
    } else {
        // Show the status bar
        m_statusBar->show();
    }
}

// Update the status bar
void FileManagerMainWindow::updateStatusBar()
{
    // Print the name of the called function
    qDebug() << Q_FUNC_INFO;

    // Get the selected indexes
    QModelIndexList selectedIndexes = m_treeView->selectionModel()->selectedIndexes();

    // Calculate the size of the selected items on disk
    qint64 size = 0;
    for (const QModelIndex &index : selectedIndexes) {
        size += m_fileSystemModel->size(index);
    }

    // Format the size in a human-readable format using the user's locale settings
    QString sizeString = QLocale().formattedDataSize(size);

    // Show the number of selected items and their size on disk in the status bar
    m_statusBar->showMessage(
            QString("%1 items selected (%2)").arg(selectedIndexes.size()).arg(sizeString));
}

// Getter method for the directory property
QString FileManagerMainWindow::directory() const
{
    return m_currentDir;
}

// Setter method for the directory property
void FileManagerMainWindow::setDirectory(const QString &directory)
{
    m_currentDir = directory;
}

void FileManagerMainWindow::openFolderInNewWindow(const QString &rootPath)
{
    // Check if the path exists, show an error dialog if it is not
    if (!QDir(rootPath).exists()) {
        QMessageBox::critical(nullptr, "Error", "This folder does not exist.");
        return;
    }

    // Check if the path exists and is a directory or a symlink to a directory, show an error dialog
    // if it is not
    if (!QFileInfo(rootPath).exists() || !QFileInfo(rootPath).isDir()) {
        QMessageBox::critical(nullptr, "Error", "This path is not a folder.");
        return;
    }

    // Check if the path is readable, show an error dialog if it is not
    if (!QFileInfo(rootPath).isReadable()) {
        QMessageBox::critical(nullptr, "Error", "This path is not readable.");
        return;
    }

    QString resolvedRootPath = rootPath;

    // If it is a symlink, resolve it
    if (QFileInfo(rootPath).isSymLink()) {
        resolvedRootPath = QFileInfo(rootPath).symLinkTarget();
    }

    // Get the normalized absolute path
    resolvedRootPath = QDir(resolvedRootPath).canonicalPath();

    // Check if a window for the specified root path already exists
    bool windowExists = false;
    for (FileManagerMainWindow *window : instances()) {
        if (window->m_fileSystemModel->rootPath() == rootPath) {
            // A window for the specified root path already exists, so raise it and return
            window->raise();
            windowExists = true;
            break;
        }
    }

    if (!windowExists) {
        // No window for the specified root path exists, so create a new one
        // Not setting a parent, so that the window does not get destroyed when the parent gets
        // destroyed
        FileManagerMainWindow *newWindow = new FileManagerMainWindow(nullptr, rootPath);
        newWindow->show();
    }
}

void FileManagerMainWindow::copySelectedItems(const QString &destinationPath)
{
    // Print the name of the called function
    qDebug() << Q_FUNC_INFO;
}

void FileManagerMainWindow::moveSelectedItems(const QString &destinationPath)
{
    // Print the name of the called function
    qDebug() << Q_FUNC_INFO;
}

void FileManagerMainWindow::open(const QString &filePath)
{
    // Print the name of the called function
    qDebug() << Q_FUNC_INFO;

    // Get the index of the selected item in the tree or list view
    const QModelIndex selectedIndex = m_selectionModel->currentIndex();

    if (selectedIndex.isValid())
    {
        // Check if we are in the tree view or the icon view
        QAbstractItemView* view;

        if (view)
        {
            // Get the item delegate for the selected index
            qDebug() << "Getting the item delegate for the selected index";

            // Get the index of the selected item in the icon view
            const QModelIndex selectedIndex = m_iconView->currentIndex();

            // Get the item delegate for the selected index
            QAbstractItemDelegate* itemDelegate = m_iconView->getItemDelegateForIndex(selectedIndex);

            qDebug() << "itemDelegate:" << itemDelegate;

            // Check if the item delegate is an instance of CustomItemDelegate
            CustomItemDelegate* customDelegate = dynamic_cast<CustomItemDelegate*>(itemDelegate);
            qDebug() << "customDelegate:" << customDelegate;

            if (customDelegate)
            {
                // Stop the previous animation, if any, and start the new animation
                qDebug() << "Stopping the previous animation, if any, and starting the new animation";
                customDelegate->stopAnimation();
                if (selectedIndex.isValid())
                {
                    qDebug() << "Starting animation for selected index:" << selectedIndex;
                    customDelegate->startAnimation(selectedIndex);
                }
                else
                {
                    qDebug() << "Invalid selected index. Cannot start animation.";
                }
            } else {
                qDebug() << "ERROR: The item delegate is not an instance of CustomItemDelegate";
            }
        } else {
            qDebug() << "ERROR: The view is null";
        }
    }

    // Check if filePath ends with ".app" or ".AppDir" or ".AppImage" or ".desktop"
    ApplicationBundle bundle(filePath);
    if (bundle.isValid()) {
        // Use the "launch" command to open the file
        qDebug() << "Launching:" << filePath;
        QProcess process;
        process.setProgram("launch");
        process.setArguments({ filePath });
        process.startDetached();
    } else {

        // Check if the filePath is a directory or a file
        if (QFileInfo(filePath).isDir()) {
            QString rootPath = m_fileSystemModel->filePath(selectedIndex);
            // If central widget is a tree view, open folder in existing window; else open in new
            // window
            if (m_treeView->isVisible()) {
                // Let the tree view do its thing
            } else {
                // Check if we already have a window open for the folder
                if (instanceExists(filePath)) {
                    // Get the window
                    qDebug() << "Window already exists:" << filePath;
                    FileManagerMainWindow* window = getInstanceForDirectory(filePath);
                    // Activate the window
                    window->activateWindow();
                    window->raise();
                    // If it is minimized, restore it
                    if (window->isMinimized()) {
                        window->showNormal();
                    }
                } else {
                    // If we don't, open a new window
                    qDebug() << "Opening new window:" << filePath;
                    openFolderInNewWindow(rootPath);
                }
            }
        } else {
            // Use the "open" command to open the file
            qDebug() << "Opening:" << filePath;
            QProcess process;
            process.setProgram("open");
            process.setArguments({ filePath });
            process.startDetached();
        }
    }
}

QString FileManagerMainWindow::getPath() const {
    return m_currentDir;
}

FileManagerMainWindow* FileManagerMainWindow::getInstanceForDirectory(const QString &directory)
{
    // Print the name of the called function
    qDebug() << Q_FUNC_INFO;

    // Iterate over the list of instances
    for (FileManagerMainWindow* instance : instances() ) {
        // Check if the instance's directory is the same as the given directory
        if (instance->getPath() == directory) {
            // Return the instance
            return instance;
        }
    }

    // Return null if no instance was found
    return nullptr;
}

void FileManagerMainWindow::openWith(const QString &filePath)
{
    // Print the name of the called function
    qDebug() << Q_FUNC_INFO;

    // Use the "open" command to open the file
    QProcess process;
    process.setProgram("open");
    process.setArguments({ "--chooser", filePath });
    process.startDetached();
}

/* This function will update the name of the selected item in the file system,
 * as well as in the tree or list view.
 */
void FileManagerMainWindow::renameSelectedItem()
{
    // Print the name of the called function
    qDebug() << Q_FUNC_INFO;

    // Get the index of the selected item in the tree or list view
    const QModelIndex selectedIndex = m_selectionModel->currentIndex();

    // Get the current name of the selected item
    const QString currentName = m_fileSystemModel->fileName(selectedIndex);

    // Prevent the user from using "/" anywhere in the new name
    QRegExpValidator validator(QRegExp("[^/]*"));
    bool ok;

    QLineEdit* lineEdit = new QLineEdit();
    lineEdit->setValidator(&validator);
    // Construct a dialog using this QLineEdit
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Rename"));
    dialog.setLayout(new QVBoxLayout());
    dialog.layout()->addWidget(lineEdit);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    dialog.layout()->addWidget(buttonBox);
    dialog.adjustSize();
    dialog.setFixedWidth(400);
    lineEdit->setText(currentName);
    lineEdit->selectAll();
    lineEdit->setFocus();
    ok = dialog.exec();

    // Get the new name of the selected item
    const QString newName = lineEdit->text();

    if (!ok || newName.isEmpty() || newName == currentName) {
        // The user canceled the dialog or didn't enter a new name
        return;
    }

    // Rename the selected item in the file system
    const QString currentPath = m_fileSystemModel->filePath(selectedIndex);
    const QString newPath = currentPath.left(currentPath.lastIndexOf("/") + 1) + newName;
    if(!QFile::rename(currentPath, newPath)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not rename %1 to %2").arg(currentPath).arg(newPath));
    } else {
        qDebug() << "Renamed" << currentPath << "to" << newPath;
        // The view will automatically update itself; works
    }
}

void FileManagerMainWindow::updateMenus()
{
    // Print the name of the called function
    qDebug() << Q_FUNC_INFO;

    // Get the list of selected indexes
    const QModelIndexList selectedIndexes = m_selectionModel->selectedIndexes();

    // Check if there is exactly one selected item
    if (selectedIndexes.size() == 1) {
        // Get the selected index
        const QModelIndex selectedIndex = selectedIndexes.first();
        m_renameAction->setEnabled(true);
    } else {
        // Disable the Rename action
        m_renameAction->setEnabled(false);
    }

    // If not at least one item is selected, disable the Open and Open With actions
    if (selectedIndexes.isEmpty()) {
        m_openAction->setEnabled(false);
        m_openWithAction->setEnabled(false);
        m_moveToTrashAction->setEnabled(false);
    } else {
        m_openAction->setEnabled(true);
        m_openWithAction->setEnabled(true);
        m_moveToTrashAction->setEnabled(true);
    }
}

QStringList FileManagerMainWindow::readFilenamesFromHiddenFile(const QString &filePath)
{
    QStringList filenames;

    // Open the .hidden file for reading
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        // Read the filenames from the file and store them in the list
        QTextStream stream(&file);
        while (!stream.atEnd()) {
            QString filename = stream.readLine().trimmed();
            if (!filename.isEmpty()) {
                filenames << filename;
            }
        }

        // Close the file
        file.close();
    }

    qDebug() << "Hidden files:" << filenames;

    return filenames;
}

bool FileManagerMainWindow::instanceExists(const QString &directory)
{
    // static QList<FileManagerMainWindow *> & instances();
    for (FileManagerMainWindow *instance : instances()) {
        if (instance->m_currentDir == directory) {
            return true;
        }
    }
    return false;
}

QWidget* FileManagerMainWindow::getCurrentView() const
{
    QWidget* currentActiveView = nullptr;

    // Check which view is currently visible in the stacked widget
    if (m_stackedWidget->currentWidget() == m_treeView)
    {
        currentActiveView = m_treeView;
    }
    else if (m_stackedWidget->currentWidget() == m_iconView)
    {
        currentActiveView = m_iconView;
    }

    return currentActiveView;
}

bool FileManagerMainWindow::isFirstInstance() const
{
    return m_isFirstInstance;
}