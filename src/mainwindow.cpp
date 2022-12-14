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

#include "mainwindow.h"
#include "customitemdelegate.h"
#include "customfileiconprovider.h"
#include "extendedattributes.h"

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

/*
 * This creates a FileManagerMainWindow object with a QTreeView and QListView widget.
 * The QTreeView widget displays the file system hierarchy, and the QListView widget displays
 * the files and directories in the selected directory. The QFileSystemModel class is used
 * to provide data for the views. The QItemSelectionModel class is used to manage the selection
 * of items in the views. The QMenuBar and QMenu classes are used to create a menu bar with a menu.
 * The menu has actions for switching between the tree and icon views.
 * The QStatusBar class is used to create a status bar that shows the number of items selected in the views.
 */

QList<FileManagerMainWindow*> &FileManagerMainWindow::instances()
{
    static QList<FileManagerMainWindow*> instances;
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
        m_is_first_instance = true;
        // If this is the first window, set this window as the root window of the main screen
        // Check if ~/Desktop exists; create it if it doesn't
        QDir homeDir(QDir::homePath());
        if (!homeDir.exists("Desktop")) {
            homeDir.mkdir("Desktop");
        }
        // Set the root path to ~/Desktop
        setDirectory(QDir::homePath() + "/Desktop");
        qDebug() << "First instance, show the desktop";
        QString desktopPicture = "/usr/local/share/slim/themes/default/background.jpg";
        // Check if the desktop picture exists and is readable, show a waning dialog if it isn't
        if (!QFile::exists(desktopPicture) || !QFile::permissions(desktopPicture).testFlag(QFile::ReadUser)) {
            QMessageBox::warning(this, "Desktop picture not found", "The desktop picture could not be found or is not readable.");
            setStyleSheet("background-color: grey;");
        } else {
            // Set the desktop picture
            setStyleSheet("border-image: url(" + desktopPicture + ") 0 0 0 0 stretch stretch;");
        }

        setAutoFillBackground(true);
        setWindowFlags(Qt::FramelessWindowHint);
        setFixedSize(QApplication::desktop()->screenGeometry(0).size());
        setAttribute(Qt::WA_X11NetWmWindowTypeDesktop, true);
    } else {
        m_is_first_instance = false;
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
            QRect positionAndGeometryRect = QRect(positionAndGeometryList[0].toInt(),
                                                  positionAndGeometryList[1].toInt(),
                                                  positionAndGeometryList[2].toInt(),
                                                  positionAndGeometryList[3].toInt());
            qDebug() << "positionAndGeometryRect:" << positionAndGeometryRect;
            setGeometry(positionAndGeometryRect);
        } else {
            qDebug() << "positionAndGeometryString is invalid";
            resize(600, 400);
            // move(100, 100);
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
    m_iconView = new QListView(this);

    // Add the tree view and list view to the stacked widget
    m_stackedWidget->addWidget(m_treeView);
    m_stackedWidget->addWidget(m_iconView);

    // Set the stacked widget as the central widget
    setCentralWidget(m_stackedWidget);

    // No frame around the views
    m_treeView->setFrameStyle(QFrame::NoFrame);
    m_iconView->setFrameStyle(QFrame::NoFrame);

    // Create an instance of our custom QFileIconProvider
    CustomFileIconProvider provider;

    m_fileSystemModel = new QFileSystemModel(this);

    // Make the file system model use the custom icon provider
    m_fileSystemModel->setIconProvider(&provider);

    // Filter out items that should not be shown
    m_fileSystemModel->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot ); // | QDir::Hidden

    // Make it use the icon provider on files as well
    m_fileSystemModel->setResolveSymlinks(true);


    // A reliable way to hide hidden files seems to be to construct a QStringList with the names of the
    // items that should be shown; doing it this way breaks the tree view, which can then no longer be expanded
    /*
    QDir dir(m_currentDir);
    QStringList itemsToBeShown = dir.entryList();
    QStringList hiddenItems;

    // Read the lines from the file .hidden into the QStringList hiddenItems
    QFile file(m_currentDir + "/.hidden");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            hiddenItems.append(line);
        }
    }

    // Remove the hidden items from the list of items to be shown
    for (int i = 0; i < hiddenItems.count(); i++) {
        itemsToBeShown.removeAll(hiddenItems[i]);
    }

    m_fileSystemModel->setNameFilters(itemsToBeShown);

    // This is needed so that the disabled items are not shown;
    // otherwise, the disabled items are shown but cannot be used
    m_fileSystemModel->setNameFilterDisables(false);
    */

    // Set the file system model as the model for the tree view and icon view
    m_treeView->setModel(m_fileSystemModel);
    m_iconView->setModel(m_fileSystemModel);

    // Set the root index to the model's root directory
    m_treeView->setRootIndex(m_fileSystemModel->index(m_currentDir));
    m_iconView->setRootIndex(m_fileSystemModel->index(m_currentDir));

    // Set the root path to the specified m_currentDir
    m_fileSystemModel->setRootPath(m_currentDir);

    // Set the window title to the root path of the QFileSystemModel
    setWindowTitle(QFileInfo(m_fileSystemModel->rootPath()).fileName());

    // Create an instance of the CustomItemDelegate class;
    // we need this so that we have control over how the items (icons with text)
    // get drawn

    CustomItemDelegate *customItemDelegate = new CustomItemDelegate(this, m_fileSystemModel);

    m_iconView->setItemDelegate(customItemDelegate);
    m_treeView->setItemDelegate(customItemDelegate);

    // Create the selection model for the tree view and icon view
    m_selectionModel = new QItemSelectionModel(m_fileSystemModel);

    // Set the selection model for the tree view and icon view
    m_treeView->setSelectionModel(m_selectionModel);
    m_iconView->setSelectionModel(m_selectionModel);

    // Enable drag-and-drop
    m_treeView->setDragEnabled(true);
    m_iconView->setDragEnabled(true);
    m_treeView->setAcceptDrops(true);
    m_iconView->setAcceptDrops(true);

    // Enable dropping on folders that are writable by the user
    m_treeView->setDropIndicatorShown(true);
    m_iconView->setDropIndicatorShown(true);
    connect(m_treeView, &QTreeView::entered, this, &FileManagerMainWindow::enableDropOnFolder);
    connect(m_iconView, &QListView::entered, this, &FileManagerMainWindow::enableDropOnFolder);

    // Create the menu bar
    m_menuBar = new QMenuBar(this);

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
    connect(m_selectionModel, &QItemSelectionModel::selectionChanged, this, &FileManagerMainWindow::updateStatusBar);
    updateStatusBar();

    // To call the updateMenus() function whenever the selected items change,
    // connect the QItemSelectionModel's selectionChanged() signal to the updateMenus() slot
    connect(m_selectionModel, &QItemSelectionModel::selectionChanged, this, &FileManagerMainWindow::updateMenus);
    updateMenus();

    // Connect the doubleClicked() signal to the open() slot
    connect(m_iconView, &QTreeView::doubleClicked, this, [this] (const QModelIndex &index)
    {
        QString filePath = m_fileSystemModel->filePath(index);
        open(filePath);
    }, Qt::QueuedConnection);

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
    }

    // Set the icon size to 32x32 pixels
    m_iconView->setIconSize(QSize(32, 32));
    m_treeView->setIconSize(QSize(16, 16));

    // Set the view mode to IconMode with the text under the icons and make the icons freely movable
    m_iconView->setViewMode(QListView::IconMode);
    m_iconView->setMovement(QListView::Free);

    // Put the icons on a grid
    QSize iconSize = m_iconView->iconSize();
    m_iconView->setGridSize(QSize(iconSize.width() * 5, iconSize.height() * 2));

    // Connect the doubleClicked() signal to the open() slot
    connect(m_treeView, &QTreeView::doubleClicked, this, [this] (const QModelIndex &index)
    {
        QString filePath = m_fileSystemModel->filePath(index);
        open(filePath);
    }, Qt::QueuedConnection);

    /* Overall */

    // showTreeView();
    // showIconView();

    // Read extended attribute describing the view mode
    QByteArray viewMode = extendedAttributes.read("WindowView");
    int viewModeInt = viewMode.toInt();
    qDebug() << "viewModeString:" << viewModeInt;
    if(viewModeInt == 1) {
        // Set the central widget to the list view (icons)
        showTreeView();
    } else {
        // Set the central widget to the tree view
        showIconView();
    }

    // Call destructor and destroy the window immediately when the window is closed
    // Only this way the window will be destroyed immediately and not when the event loop is finished
    // and we can remove the window from the list of child windows of the parent window
    setAttribute(Qt::WA_DeleteOnClose);
}

// Saves the window geometry
void FileManagerMainWindow::saveWindowGeometry()
{
// Print window positionAndGeometry
    qDebug() << "Window positionAndGeometry: " << geometry();
    qDebug() << "Window size: " << size();
    qDebug() << "Window position: " << pos();

    // Get the number of the display the window is on
    int displayNumber = QGuiApplication::screens().indexOf(windowHandle()->screen());
    qDebug() << "Window display number: " << displayNumber;

    // Get the filename of the current directory
    QString currentDir = m_fileSystemModel->filePath(m_fileSystemModel->index(m_fileSystemModel->rootPath()));

    // Write extended attribute to the current directory
    ExtendedAttributes ea = ExtendedAttributes(currentDir);

    // Writing the window position and geometry directly as a QByteArray does not work because it contains null bytes, so we convert it to a string
    QString positionAndGeometry = QString::number(pos().x()) + "," + QString::number(pos().y()) + "," + QString::number(geometry().width()) + "," + QString::number(geometry().height());
    QByteArray positionAndGeometryByteArray = positionAndGeometry.toUtf8();

    // Write the window positionAndGeometry to an extended attribute
    ea.write("positionAndGeometry", positionAndGeometryByteArray);

    // If "Tree View" is checked in the "View" menu, write "1" to the extended attribute,
    // otherwise write "2" to the extended attribute
    if (m_treeViewAction->isChecked())
    {
        ea.write("WindowView", "1");
    }
    else
    {
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
    if (! m_treeViewAction->isChecked()) {
        m_iconView->doItemsLayout();
    }

    // TOOD: Wait until no resize events are coming in for 1 second
    // This is necessary because the resizeEvent() is called multiple times when the user resizes the window
    // and we only want to save the window geometry when the user has finished resizing the window.
    // How to do this properly?

    saveWindowGeometry();
}

FileManagerMainWindow::~FileManagerMainWindow()
{
    qDebug() << "Destructor called";

    // Save the window geometry
    saveWindowGeometry();

    // Remove from the list of windows
    instances().removeAll(this);

    // If this is the last window, quit the application
    if (instances().isEmpty())
    {
        qDebug() << "Last window closed, quitting application";
        qApp->quit();
    }

}

void FileManagerMainWindow::createMenus()
{
    // Create the File menu
    QMenu *fileMenu = new QMenu(tr("File"));

    // Add the usual menu items to the File menu
    fileMenu->addAction(tr("New..."));
    fileMenu->actions().last()->setShortcut(QKeySequence("Ctrl+N"));
    fileMenu->actions().last()->setEnabled(false);

    fileMenu->addAction(tr("Open"));
    fileMenu->actions().last()->setShortcut(QKeySequence("Ctrl+O"));
    connect(fileMenu->actions().last(), &QAction::triggered, this, [this] ()
    {
        QModelIndex index = m_treeView->currentIndex();
        QString filePath = m_fileSystemModel->filePath(index);
        open(filePath);
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
    if (m_is_first_instance) {
        fileMenu->actions().last()->setEnabled(false);
    }

    fileMenu->addSeparator();

    QAction* closeAction = new QAction(tr("Close"), this);
    closeAction->setShortcut(QKeySequence("Ctrl+W"));
    fileMenu->addAction(closeAction);
    connect(closeAction, &QAction::triggered, this, &FileManagerMainWindow::close);
    if(m_is_first_instance) {
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
    editMenu->addAction(tr("Delete"));
    editMenu->actions().last()->setShortcut(QKeySequence("Ctrl-Backspace"));
    editMenu->addSeparator();
    QAction* selectAllAction = new QAction(tr("Select All"), this);
    selectAllAction->setShortcut(QKeySequence("Ctrl+A"));
    editMenu->addAction(selectAllAction);

    // Connect the triggered() signal of the Select All action to a slot
    connect(selectAllAction, &QAction::triggered, this, &FileManagerMainWindow::selectAll);

    // Create the Rename action
    m_renameAction = editMenu->addAction(tr("Rename..."), this, &FileManagerMainWindow::renameSelectedItem);


    // Set the shortcut key for the Rename action to Ctrl+R
    m_renameAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));

    editMenu->addAction(m_renameAction);

    // Add the Edit menu to the menu bar
    m_menuBar->addMenu(editMenu);

    // Create the View menu
    QMenu* viewMenu = new QMenu(tr("View"));

    // Connect the triggered() signals of the Tree View and Icon View actions to slots
    connect(m_treeViewAction, &QAction::triggered, this, &FileManagerMainWindow::showTreeView);
    connect(m_iconViewAction, &QAction::triggered, this, &FileManagerMainWindow::showIconView);

    // Add the Tree View and Icon View actions to the View menu
    viewMenu->addAction(m_treeViewAction);
    if (m_is_first_instance)
        viewMenu->actions().last()->setEnabled(false);
    viewMenu->addAction(m_iconViewAction);
    if (m_is_first_instance)
        viewMenu->actions().last()->setEnabled(false);

    // Create the Show/Hide Status Bar action
    QAction* showHideStatusBarAction = new QAction(tr("Show/Hide Status Bar"), this);

    // Add the Show/Hide Status Bar action to the View menu
    viewMenu->addAction(showHideStatusBarAction);

    // Connect the triggered() signal of the Show/Hide Status Bar action to a slot
    connect(showHideStatusBarAction, &QAction::triggered, this, &FileManagerMainWindow::showHideStatusBar);

    // Add the View menu to the menu bar
    m_menuBar->addMenu(viewMenu);

    // Create the Go menu
    QMenu* goMenu = new QMenu(tr("Go"));

    goMenu->addAction(tr("Go Up"));
    goMenu->actions().last()->setShortcut(QKeySequence("Ctrl+Up"));
    if(QFileInfo(m_currentDir).canonicalFilePath() == "/") {
        goMenu->actions().last()->setEnabled(false);
    }
    connect(goMenu->actions().last(), &QAction::triggered, this, [this](){
        openFolderInNewWindow(QFileInfo(m_currentDir + "/../").canonicalFilePath());
    });
    if (m_is_first_instance)
        goMenu->actions().last()->setEnabled(false);

    goMenu->addAction(tr("Go Up and Close Current"));
    goMenu->actions().last()->setShortcut(QKeySequence("Shift+Ctrl+Up"));
    if(QFileInfo(m_currentDir).canonicalFilePath() == "/") {
        goMenu->actions().last()->setEnabled(false);
    }
    connect(goMenu->actions().last(), &QAction::triggered, this, [this](){
        openFolderInNewWindow(QFileInfo(m_currentDir + "/../").canonicalFilePath());
        close();
    });
    if (m_is_first_instance)
        goMenu->actions().last()->setEnabled(false);

    goMenu->addSeparator();

    goMenu->addAction(tr("Computer"));
    goMenu->actions().last()->setShortcut(QKeySequence("Ctrl+Shift+C"));
    connect(goMenu->actions().last(), &QAction::triggered, this, [this](){
        openFolderInNewWindow("/");
    });

    goMenu->addAction(tr("Network"));
    goMenu->actions().last()->setShortcut(QKeySequence("Ctrl+Shift+N"));
    connect(goMenu->actions().last(), &QAction::triggered, this, [this](){
        QMessageBox::information(nullptr, (" "), tr("This feature is still in development."));
    });

    goMenu->addAction(tr("Devices"));
    goMenu->actions().last()->setShortcut(QKeySequence("Ctrl+Shift+U"));
    connect(goMenu->actions().last(), &QAction::triggered, this, [this](){
        openFolderInNewWindow("/media");
    });
    goMenu->addAction(tr("Applications"));
    goMenu->actions().last()->setShortcut(QKeySequence("Ctrl+Shift+A"));
    connect(goMenu->actions().last(), &QAction::triggered, this, [this](){
        openFolderInNewWindow("/Applications");
    });

    goMenu->addSeparator();

    goMenu->addAction(tr("Home"));
    goMenu->actions().last()->setShortcut(QKeySequence("Ctrl+Home"));
    connect(goMenu->actions().last(), &QAction::triggered, this, [this](){
        openFolderInNewWindow(QDir::homePath());
    });

    goMenu->addAction(tr("Documents"));
    goMenu->actions().last()->setShortcut(QKeySequence("Ctrl+Shift+D"));
    connect(goMenu->actions().last(), &QAction::triggered, this, [this](){
        // Create if it doesn't exist
        QDir dir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
        if (!dir.exists())
            dir.mkpath(".");
        openFolderInNewWindow(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    });

    goMenu->addAction(tr("Downloads"));
    goMenu->actions().last()->setShortcut(QKeySequence("Ctrl+Shift+L"));
    connect(goMenu->actions().last(), &QAction::triggered, this, [this](){
        // Create if it doesn't exist
        QDir dir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
        if (!dir.exists())
            dir.mkpath(".");
        openFolderInNewWindow(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
    });

    goMenu->addAction(tr("Music"));
    goMenu->actions().last()->setShortcut(QKeySequence("Ctrl+Shift+M"));
    connect(goMenu->actions().last(), &QAction::triggered, this, [this](){
        // Create if it doesn't exist
        QDir dir(QStandardPaths::writableLocation(QStandardPaths::MusicLocation));
        if (!dir.exists())
            dir.mkpath(".");
        openFolderInNewWindow(QStandardPaths::writableLocation(QStandardPaths::MusicLocation));
    });

    goMenu->addAction(tr("Pictures"));
    goMenu->actions().last()->setShortcut(QKeySequence("Ctrl+Shift+P"));
    connect(goMenu->actions().last(), &QAction::triggered, this, [this](){
        // Create if it doesn't exist
        QDir dir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
        if (!dir.exists())
            dir.mkpath(".");
        openFolderInNewWindow(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    });

    goMenu->addAction(tr("Videos"));
    goMenu->actions().last()->setShortcut(QKeySequence("Ctrl+Shift+V"));
    connect(goMenu->actions().last(), &QAction::triggered, this, [this](){
        // Create if it doesn't exist
        QDir dir(QStandardPaths::writableLocation(QStandardPaths::MoviesLocation));
        if (!dir.exists())
            dir.mkpath(".");
        openFolderInNewWindow(QStandardPaths::writableLocation(QStandardPaths::MoviesLocation));
    });

    goMenu->addSeparator();

    goMenu->addAction(tr("Trash"));
    goMenu->actions().last()->setShortcut(QKeySequence("Ctrl+Shift+T"));
    connect(goMenu->actions().last(), &QAction::triggered, this, [this](){
        // XDG Trash path
        openFolderInNewWindow(QDir::homePath() + "/.local/share/Trash/files");
    });

    // Add the Go menu to the menu bar
    m_menuBar->addMenu(goMenu);

    // Create the Help menu
    QMenu* helpMenu = new QMenu("Help");

    // Add an action to the Help menu
    QAction *a = helpMenu->addAction("About Filer");

    // Add the Help menu to the menu bar
    m_menuBar->addMenu(helpMenu);

    // Connect the triggered() signal of the About menu action to a slot
    connect(a, &QAction::triggered, this, &FileManagerMainWindow::showAboutBox);

    // Set up additional shortcuts

    QShortcut *shortcut;

    shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Down), this);
    connect(shortcut, &QShortcut::activated, this, [this] (){
        QModelIndex index = m_treeView->currentIndex();
        QString filePath = m_fileSystemModel->filePath(index);
        open(filePath);
    }, Qt::QueuedConnection);

    shortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::CTRL + Qt::Key_Down), this);
    connect(shortcut, &QShortcut::activated, this, [this] (){
        QModelIndex index = m_treeView->currentIndex();
        QString filePath = m_fileSystemModel->filePath(index);
        openWith(filePath);
    }, Qt::QueuedConnection);

    shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Down), this);
    connect(shortcut, &QShortcut::activated, this, [this] (){
        QModelIndex index = m_treeView->currentIndex();
        QString filePath = m_fileSystemModel->filePath(index);
        open(filePath);
        if(!m_is_first_instance)
            close();
    }, Qt::QueuedConnection);

    /*
     * TODO
    shortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    connect(shortcut, &QShortcut::activated, this, &MainWindow::on_actionDelete_triggered);

    shortcut = new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Delete), this);
    connect(shortcut, &QShortcut::activated, this, &MainWindow::on_actionDeleteWithoutTrash_triggered);

    shortcut = new QShortcut(QKeySequence(Qt::SHIFT + Qt::CTRL + Qt::Key_Backspace), this);
    connect(shortcut, &QShortcut::activated, this, &MainWindow::on_actionDeleteWithoutTrash_triggered);
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
    // Get the parent index of the current index
    QModelIndex parentIndex = m_treeView->currentIndex().parent();

    // Create a new selection model
    QItemSelectionModel* selectionModel = new QItemSelectionModel(m_treeView->model());

    // Set the selection model for the tree view
    m_treeView->setSelectionModel(selectionModel);

    // Select all items in the parent directory
    selectionModel->select(QItemSelection(parentIndex.child(0, 0), parentIndex.child(parentIndex.model()->rowCount() - 1, 0)), QItemSelectionModel::Select);
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
    for (const QModelIndex& index : selectedIndexes) {
        size += m_fileSystemModel->size(index);
    }

    // Format the size in a human-readable format using the user's locale settings
    QString sizeString = QLocale().formattedDataSize(size);

    // Show the number of selected items and their size on disk in the status bar
    m_statusBar->showMessage(QString("%1 items selected (%2)").arg(selectedIndexes.size()).arg(sizeString));
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

// Open a folder in the current window
void FileManagerMainWindow::openFolder(const QString &rootPath)
{
    // Print the name of the called function
    qDebug() << Q_FUNC_INFO;


    // Set the root path to the selected directory
    m_fileSystemModel->setRootPath(rootPath);

    // Set the root index to the selected directory
    m_treeView->setRootIndex(m_fileSystemModel->index(rootPath));

}

void FileManagerMainWindow::openFolderInNewWindow(const QString &rootPath)
{
    // Check if the path exists, show an error dialog if it is not
    if (!QDir(rootPath).exists()) {
        QMessageBox::critical(nullptr, "Error", "This folder does not exist.");
        return;
    }

    // Check if the path exists and is a directory or a symlink to a directory, show an error dialog if it is not
    if (!QFileInfo(rootPath).exists() || !QFileInfo(rootPath).isDir()) {
        QMessageBox::critical(nullptr, "Error", "This path is not a folder.");
        return;
    }

    // Check if the path is readable, show an error dialog if it is not
    if (!QFileInfo(rootPath).isReadable()) {
        QMessageBox::critical(nullptr, "Error", "This path is not readable.");
        return;
    }

    // Check if a window for the specified root path already exists
    bool windowExists = false;
    for (FileManagerMainWindow* window : instances()) {
        if (window->m_fileSystemModel->rootPath() == rootPath) {
            // A window for the specified root path already exists, so raise it and return
            window->raise();
            windowExists = true;
            break;
        }
    }

    if (!windowExists) {
        // No window for the specified root path exists, so create a new one
        // Not setting a parent, so that the window does not get destroyed when the parent gets destroyed
        FileManagerMainWindow* newWindow = new FileManagerMainWindow(nullptr, rootPath);
        newWindow->show();
    }
}

void FileManagerMainWindow::onItemDropped(const QModelIndex &targetIndex)
{
    // Check if the target index is valid and if it is a directory
    if (!targetIndex.isValid() || !m_fileSystemModel->isDir(targetIndex))
        return;

    // Create the context menu
    QMenu contextMenu;

    // Create the actions
    QAction *copyAction = contextMenu.addAction("Copy here");
    QAction *moveAction = contextMenu.addAction("Move here");

    // Execute the context menu and get the selected action
    QAction *selectedAction = contextMenu.exec(QCursor::pos());

    // Get the file path of the target index
    QString targetPath = m_fileSystemModel->filePath(targetIndex);

    // Handle the selected action
    if (selectedAction == copyAction)
    {
        // Copy the selected items to the target folder
        copySelectedItems(targetPath);
    }
    else if (selectedAction == moveAction)
    {
        // Move the selected items to the target folder
        moveSelectedItems(targetPath);
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

/* This function checks if the specified index is valid, if the item at the index is a directory,
 * and if the directory is writable by the user. If all these conditions are met,
 * it enables the drop indicator for the tree view and the icon view, allowing
 * the user to drop items onto the folder.
 * Otherwise, it disables the drop indicator, preventing the user from dropping items onto the folder.
 */
void FileManagerMainWindow::enableDropOnFolder(const QModelIndex &index)
{
    /*
     * This code is commented out because it made the TreeView crash when the user used the scroll wheel
     *
    // Print the name of the called function
    qDebug() << Q_FUNC_INFO;

    if (index.isValid() && m_fileSystemModel->isDir(index))
    {
        // Get the permissions for the folder
        QFile::Permissions permissions = m_fileSystemModel->permissions(index);

        // Check if the WriteUser flag is set
        if (permissions & QFile::WriteUser) {
            // The folder is writable by the user
            qDebug() << "The folder is writable by the user";
            m_treeView->setDropIndicatorShown(true);
            m_iconView->setDropIndicatorShown(true);

            return;
        }
    }

    m_treeView->setDropIndicatorShown(false);
    m_iconView->setDropIndicatorShown(false);
    qDebug() << "The folder is not writable by the user";
     */
}

void FileManagerMainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    // Print the name of the called function
    qDebug() << Q_FUNC_INFO;

    // Get the model index of the item the drag is over
    const QModelIndex targetIndex = m_iconView->indexAt(event->pos());

    // Create a QFileInfo object for the target folder
    QFileInfo targetFolderInfo(m_fileSystemModel->filePath(targetIndex));

    // Check if the target folder is writable
    if (targetFolderInfo.isWritable())
    {
        qDebug() << "The target folder is writable";
        // Accept the drag and drop event
        event->accept();

        // Show the drop indicator
        m_treeView->setDropIndicatorShown(true);
        m_iconView->setDropIndicatorShown(true);

        // Accept the event
        event->acceptProposedAction();
    }
    else
    {
        qDebug() << "The target folder is not writable";
        // Reject the event
        event->ignore();

        // Don't sShow the drop indicator
        m_treeView->setDropIndicatorShown(false);
        m_iconView->setDropIndicatorShown(false);
    }
}

void FileManagerMainWindow::dragMoveEvent(QDragMoveEvent *event)
{
    // Print the name of the called function
    qDebug() << Q_FUNC_INFO;

    // Get the model index of the item the drag is over
   const QModelIndex targetIndex = m_iconView->indexAt(event->pos());

    // Get the file path of the item
    const QString filePath = m_fileSystemModel->filePath(targetIndex);

    // Create a QFileInfo object for the item
    QFileInfo fileInfo(filePath);

    // Check if the item is a folder and is writable by the user
    if (fileInfo.isDir() && fileInfo.isWritable())
    {
        // Accept the drag and drop event
        event->accept();

        // Show the drop indicator
        m_treeView->setDropIndicatorShown(true);
        m_iconView->setDropIndicatorShown(true);
    }
    else
    {
        // Ignore the drag and drop event
        event->ignore();

        // Don't sShow the drop indicator
        m_treeView->setDropIndicatorShown(false);
        m_iconView->setDropIndicatorShown(false);
    }
}


void FileManagerMainWindow::open(const QString &filePath)
{
    // Print the name of the called function
    qDebug() << Q_FUNC_INFO;

    // Get the index of the selected item in the tree or list view
    const QModelIndex selectedIndex = m_selectionModel->currentIndex();

    // Check if the file path ends with ".app", ".AppDir", or ".AppImage"
    if (filePath.endsWith(".app") || filePath.endsWith(".AppDir") || filePath.endsWith(".AppImage")) {
        // Use the "launch" command to open the file
        qDebug() << "Launching:" << filePath;
        QProcess process;
        process.setProgram("launch");
        process.setArguments({filePath});
        process.startDetached();
    } else {

        // Check if the filePath is a directory or a file
        if(QFileInfo(filePath).isDir())
        {
            QString rootPath = m_fileSystemModel->filePath(selectedIndex);
            // If central widget is a tree view, open folder in existing window; else open in new window
            if (this->centralWidget() == m_treeView) {
                // Open the folder in the current window
                openFolder(rootPath);
            } else {
                // Open the folder in a new window
                openFolderInNewWindow(rootPath);
            }
        }
        else
        {
            // Use the "open" command to open the file
            QProcess process;
            process.setProgram("open");
            process.setArguments({filePath});
            process.startDetached();
        }
    }
}

void FileManagerMainWindow::openWith(const QString &filePath)
{
    // Print the name of the called function
    qDebug() << Q_FUNC_INFO;

    // Use the "open" command to open the file
    QProcess process;
    process.setProgram("open");
    process.setArguments({"--chooser", filePath});
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
    const QString currentName = m_fileSystemModel->data(selectedIndex, Qt::DisplayRole).toString();

    // Use the QInputDialog class to ask the user for the new name for the selected item
    bool ok;
    const QString newName = QInputDialog::getText(nullptr, tr("Rename"), tr("New name:"), QLineEdit::Normal, currentName, &ok);
    if (!ok)
        return;

    // Use the QFileSystemModel's setData() function to rename the selected item in the file system
    m_fileSystemModel->setData(selectedIndex, newName, Qt::EditRole);
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
}

QStringList FileManagerMainWindow::readFilenamesFromHiddenFile(const QString& filePath)
{
    QStringList filenames;

    // Open the .hidden file for reading
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly))
    {
        // Read the filenames from the file and store them in the list
        QTextStream stream(&file);
        while (!stream.atEnd())
        {
            QString filename = stream.readLine().trimmed();
            if (!filename.isEmpty())
            {
                filenames << filename;
            }
        }

        // Close the file
        file.close();
    }

    qDebug() << "Hidden files:" << filenames;

    return filenames;
}
