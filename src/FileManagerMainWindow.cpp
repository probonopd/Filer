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

#include "FileOperationManager.h"

#include <QMenuBar>
#include <QMenu>
#include <QMainWindow>
#include "CustomTreeView.h"
#include <QAction>
#include <QMessageBox>
#include <QDebug>
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
#include <QColor>
#include <QSortFilterProxyModel>
#include <QMimeData>
#include <QThread>
#include <QLabel>
#include <QDialogButtonBox>
#include <QFileSystemModel>
#include <QCompleter>
#include <QRegExpValidator>
#include <QClipboard>
#include <QUrl>
#include "ApplicationBundle.h"
#include "TrashHandler.h"
#include "InfoDialog.h"
#include "AppGlobals.h"
#include "CustomProxyModel.h"
#include <QStorageInfo>
#include "Mountpoints.h"
#include <QScreen>
#include "VolumeWatcher.h"
#include <QSettings>
#include "PreferencesDialog.h"

/*
 * This creates a FileManagerMainWindow object with a QTreeView subclass and QListView subclass widget.
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

    m_extendedAttributes = new ExtendedAttributes(m_currentDir);

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

        QByteArray positionAndGeometry =  m_extendedAttributes->read("positionAndGeometry");
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
    if (m_isFirstInstance) {
        setWindowIcon(QIcon::fromTheme("user-desktop"));
    } else {
        setWindowIcon(QIcon::fromTheme("folder"));
    }

    // On FreeBSD, set padding for the desktop to leave room for the global menu bar
    // FIXME: Why is this needed on FreeBSD but not on Linux?
    // (Tested with Lubunutu 22.04 and the KDE Plasma theme)
    if (QSysInfo::kernelType() == "freebsd") {
        if (! qgetenv("UBUNTU_MENUPROXY").isEmpty() && m_isFirstInstance) {
            // Make room for the desktop with padding
            setContentsMargins(0, 22, 0, 0);
        }
    }

    // Set margins translucent
    setAttribute(Qt::WA_TranslucentBackground);

    // Initialize m_stackedWidget
    m_stackedWidget = new QStackedWidget(this);

    // Create the tree view and list view
    m_treeView = new CustomTreeView(this);
    m_iconView = new CustomListView(this);

    // Add the tree view and list view to the stacked widget
    m_stackedWidget->addWidget(m_treeView);
    m_stackedWidget->addWidget(m_iconView);

    // Set the stacked widget as the central widget
    setCentralWidget(m_stackedWidget);

    // No frame around the views
    m_treeView->setFrameStyle(QFrame::NoFrame);
    m_iconView->setFrameStyle(QFrame::NoFrame);

    // Draw the desktop picture for the first instance
    if (m_isFirstInstance) {

        // Disable scrollbars
        m_iconView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_iconView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        m_iconView->requestDesktopPictureToBePainted(true);

        // closeAllWindowsOnScreen(0); // TODO: Instead, prevent the "desktop picture only" windows created in main.cpp
        // from being available in the window menu by setting some property on them;
        // the same as we do with Installer

    // The first instance also handles screen size changes
        QList<QScreen *> screens = QGuiApplication::screens();
        for (QScreen *screen : screens) {
            connect(screen, &QScreen::geometryChanged, this, &FileManagerMainWindow::handleScreenChange);
        }
    }

    m_fileSystemModel = new CustomFileSystemModel(this);
    m_fileSystemModel->setRootPath(m_currentDir);
    m_proxyModel = new CustomProxyModel(this);
    m_proxyModel->setSourceModel(m_fileSystemModel);

    m_proxyModel->setDynamicSortFilter(true);
    m_proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);

    // Sort by type, and within types, by name
    m_proxyModel->setSortRole(Qt::DecorationRole);
    m_proxyModel->sort(0, Qt::AscendingOrder);

    // Set the file system model as the model for the tree view and icon view
    m_treeView->setModel(m_proxyModel);
    m_iconView->setModel(m_proxyModel);

    // Without this, every window just shows /
    m_treeView->setRootIndex(m_proxyModel->mapFromSource(m_fileSystemModel->index(m_currentDir)));
    m_iconView->setRootIndex(m_proxyModel->mapFromSource(m_fileSystemModel->index(m_currentDir)));

    // Set the window title to the root path of the QFileSystemModel
    setWindowTitle(QFileInfo(m_fileSystemModel->rootPath()).fileName());

    // If we are at /
    if (m_fileSystemModel->rootPath() == "/") {
        setWindowTitle(VolumeWatcher::getRootDiskName());
        // Resize the window since we cannot store the position and geometry
        // of the root window in extended attributes appropriately
        // TODO: Find a way to store the position and geometry of the root window
        resize(600, 400);
        move(40, 44); // Like the default position of windows in KWin
    }

    // If we are at the Trash, set the window title to "Trash"
    if (m_fileSystemModel->rootPath() == TrashHandler::getTrashPath()) {
        setWindowTitle(tr("Trash"));
    }

    // Create an instance of the CustomItemDelegate class;
    // we need this so that we have control over how the items (icons with text)
    // get drawn

    CustomItemDelegate *customItemDelegate = new CustomItemDelegate(m_stackedWidget->currentWidget(), m_proxyModel);

    // Install the custom item delegate as an event filter on the tree view and icon view
    // so that we can intercept mouse events like QEvent::DragMove
    m_treeView->viewport()->installEventFilter(customItemDelegate);
    m_iconView->viewport()->installEventFilter(customItemDelegate);

    m_iconView->setItemDelegate(customItemDelegate);
    m_treeView->setItemDelegate(customItemDelegate);

    // Create the selection model for the tree view and icon view
    m_selectionModel = new QItemSelectionModel(m_proxyModel, this);

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
    m_treeView->setSelectionMode(QAbstractItemView::ContiguousSelection);
    m_treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_iconView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Set the elision mode to elide ("...") the text in the middle
    m_treeView->setTextElideMode(Qt::ElideMiddle);
    m_iconView->setTextElideMode(Qt::ElideMiddle);

    // Connect the selectionChanged() signal of the selection model to a single slot
    connect(m_selectionModel, &QItemSelectionModel::selectionChanged, this,
            &FileManagerMainWindow::handleSelectionChange);

    // Call the slot immediately to initialize the UI based on the initial selection
    handleSelectionChange();

    // Connect the doubleClicked() signal to the open() slot
    connect(
            m_iconView, &QTreeView::doubleClicked, this,
            [this](const QModelIndex &index) {
                QString filePath = m_fileSystemModel->filePath(m_proxyModel->mapToSource(index));
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
        // m_iconView->setUniformItemSizes(true);
    }

    // Set the icon size to 32x32 pixels
    m_iconView->setIconSize(QSize(32, 32));
    m_treeView->setIconSize(QSize(16, 16));

    // Set the view mode to IconMode with the text under the icons
    m_iconView->setViewMode(QListView::IconMode);

    setGridSize();

    // Connect the doubleClicked() signal to the open() slot
    connect(
            m_treeView, &QTreeView::doubleClicked, this,
            [this](const QModelIndex &index) {
                qDebug() << "doubleClicked";
                QString filePath = m_fileSystemModel->filePath(m_proxyModel->mapToSource(index));
                qDebug() << "filePath:" << filePath;
                open(filePath);
            },
            Qt::QueuedConnection);

    /* Overall */

    // showTreeView();
    // showIconView();

    if (instanceCount != 0) {
        // Read extended attribute describing the view mode
        QByteArray viewMode = m_extendedAttributes->read("WindowView");
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

    // If this is the first instance, disable m_showStatusBarAction
    if (instanceCount == 0) {
        m_showStatusBarAction->setEnabled(false);
    }

    // Call destructor and destroy the window immediately when the window is closed
    // Only this way the window will be destroyed immediately and not when the event loop is
    // finished and we can remove the window from the list of child windows of the parent window
    setAttribute(Qt::WA_DeleteOnClose);

    /*
     * If no window size is set, the window will be resized so that all items fit in the window.
     * FIXME: This is not working properly yet.

    // Calculate the maximum dimensions required to accommodate all items
    int maxWidth = 0;
    int maxHeight = 0;

    // Iterate through all items in the model
    qDebug() << "m_fileSystemModel->rowCount():" << m_fileSystemModel->rowCount();
    for (int i = 0; i < m_fileSystemModel->rowCount(); ++i) {
        qDebug() << "i:" << i;
        // Get the item's x, y, width, and height
        QModelIndex index = m_fileSystemModel->index(i, 0);
        QRect rect = m_iconView->visualRect(index);
        qDebug() << "rect:" << rect;
        // Calculate the maximum width and height
        maxWidth = qMax(maxWidth, rect.x() + rect.width());
        maxHeight = qMax(maxHeight, rect.y() + rect.height());
    }
    // Set the calculated dimensions to both the QListView and QStackedWidget
    resize(maxWidth, maxHeight);
    */
}

void FileManagerMainWindow::setGridSize() {// Put the icons on a grid
    QSize iconSize = m_iconView->iconSize();
    // Get the gridSize from QSettings, default is 120
    QSettings settings;
    int gridSize = settings.value("gridSize", 120).toInt();
    qDebug() << "gridSize:" << gridSize;
    // NOTE: When the grid size is smaller than height/width of the item, the item will be cut off
    // Also, without setting any grid size, the items will not be shown at all?
    m_iconView->setGridSize(QSize(gridSize, 60));
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

    // Writing the window position and geometry directly as a QByteArray does not work because it
    // contains null bytes, so we convert it to a string
    QString positionAndGeometry = QString::number(geometry().x()) + "," + QString::number(geometry().y())
            + "," + QString::number(geometry().width()) + ","
            + QString::number(geometry().height());
    QByteArray positionAndGeometryByteArray = positionAndGeometry.toUtf8();

    // Write the window positionAndGeometry to an extended attribute
    m_extendedAttributes->write("positionAndGeometry", positionAndGeometryByteArray);

    // If "Tree View" is checked in the "View" menu, write "1" to the extended attribute,
    // otherwise write "2" to the extended attribute
    if (m_treeViewAction->isChecked()) {
        m_extendedAttributes->write("WindowView", "1");
    } else {
        m_extendedAttributes->write("WindowView", "2");
    }

    // Show global menu for Filer when it is launched
    if(m_isFirstInstance) {
        activateWindow();
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
    // qDebug() << "resizeEvent";

    // Call the base class implementation
    QMainWindow::resizeEvent(event);

    // Re-layout the items in the view to reflect the new grid layout
    if (!m_treeViewAction->isChecked()) {
        m_iconView->doItemsLayout();
    }
}

void FileManagerMainWindow::refresh() {
    qDebug() << "Calling update() on the views";
    setGridSize();
    m_treeView->update();
    // Block updating the view until all items have been moved to their custom positions in CustomListView::layoutItems()
    m_iconView->viewport()->setUpdatesEnabled(false);
    m_iconView->update();
}

FileManagerMainWindow::~FileManagerMainWindow()
{
    qDebug() << "FileManagerMainWindow::~FileManagerMainWindow()";

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

    // Call persistItemPositions(); on the source model of the icon view
    // to save the positions of the items in the icon view
    m_fileSystemModel->persistItemPositions();

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

    delete m_extendedAttributes;

    qDebug() << "FileManagerMainWindow::~FileManagerMainWindow() done";
}

void FileManagerMainWindow::createMenus()
{
    // Remove any previous menus
    menuBar()->clear();

    // Create the File menu
    QMenu *fileMenu = new QMenu(tr("File"), this);

    // Add the usual menu items to the File menu
    fileMenu->addAction(tr("New Folder..."));
    m_newAction = fileMenu->actions().last();
    m_newAction->setShortcut(QKeySequence("Ctrl+N"));
    m_newAction->setEnabled(false);
    connect(m_newAction, &QAction::triggered, this, [this]() {
        bool ok;
        // Default name = "New Folder" (translatable); if it already exists, a number is appended after a space
        QString defaultName = tr("New Folder");
        if (QFileInfo(m_currentDir + "/" + defaultName).exists()) {
            int i = 1;
            while (QFileInfo(m_currentDir + "/" + defaultName + " " + QString::number(i)).exists()) {
                i++;
            }
            defaultName += " " + QString::number(i);
        }
        QString name = QInputDialog::getText(this, tr("New Folder"), tr("Folder name:"), QLineEdit::Normal, defaultName, &ok);
        if (ok && !name.isEmpty()) {
            qDebug() << "Creating new folder " << name;
            // Get the absolute path of the current directory
            QString currentDir = m_fileSystemModel->filePath(m_fileSystemModel->index(m_fileSystemModel->rootPath()));
            // Create the new folder
            QDir dir(currentDir);
            dir.mkdir(name);
        }
    });

    fileMenu->addSeparator();

    // Open
    fileMenu->addAction(tr("Open"));
    m_openAction = fileMenu->actions().last();
    fileMenu->actions().last()->setShortcut(QKeySequence("Ctrl+O"));
    connect(fileMenu->actions().last(), &QAction::triggered, this, [this]() {
        QModelIndex index = m_treeView->currentIndex();
        // Get all selected items
        QModelIndexList selectedIndexes = getCurrentView()->selectionModel()->selectedIndexes();
        for (QModelIndex index : selectedIndexes) {
            // Get the absolute path of the item represented by the index, using the model
            QString filePath = m_fileSystemModel->data(m_proxyModel->mapToSource(index), QFileSystemModel::FilePathRole).toString();
            open(filePath);
        }
    });

    // Open With...
    fileMenu->addAction(tr("Open With..."));
    m_openWithAction = fileMenu->actions().last();
    fileMenu->actions().last()->setShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_O));
    connect(fileMenu->actions().last(), &QAction::triggered, this, [this]() {
        QModelIndex index = m_treeView->currentIndex();
        // Get all selected items
        QModelIndexList selectedIndexes = getCurrentView()->selectionModel()->selectedIndexes();
        for (QModelIndex index : selectedIndexes) {
            // Get the absolute path of the item represented by the index, using the model
            QString filePath = m_fileSystemModel->data(m_proxyModel->mapToSource(index), QFileSystemModel::FilePathRole).toString();
            openWith(filePath);
        }
    });

    // Open and close current
    fileMenu->addAction(tr("Open and close current"));
    fileMenu->actions().last()->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_O));
    connect(fileMenu->actions().last(), &QAction::triggered, [=]() {
        // Get all selected items
        QModelIndexList selectedIndexes = getCurrentView()->selectionModel()->selectedIndexes();
        for (QModelIndex index : selectedIndexes) {
            // Get the absolute path of the item represented by the index, using the model
            QString filePath = m_fileSystemModel->data(m_proxyModel->mapToSource(index), QFileSystemModel::FilePathRole).toString();
            open(filePath);
        }
        close();
    });

    if (m_isFirstInstance) {
        fileMenu->actions().last()->setEnabled(false);
    }

    fileMenu->addSeparator();

    // Show Contents
    m_showContentsAction = new QAction(tr("Show Contents"), this);
    fileMenu->addAction(m_showContentsAction);
    m_showContentsAction->setEnabled(false);
    connect(m_showContentsAction, &QAction::triggered, this, [this]() {
        QModelIndexList selectedIndexes = m_iconView->selectionModel()->selectedIndexes();
        for (QModelIndex index : selectedIndexes) {
            QString filePath = m_fileSystemModel->filePath(m_proxyModel->mapToSource(index));
            openFolderInNewWindow(filePath);
        }
    });

    fileMenu->addSeparator();

    QAction *closeAction = new QAction(tr("Close"), this);
    closeAction->setShortcut(QKeySequence("Ctrl+W"));
    fileMenu->addAction(closeAction);
    connect(closeAction, &QAction::triggered, this, &FileManagerMainWindow::close);
    if (m_isFirstInstance) {
        closeAction->setEnabled(false);
    }

    fileMenu->addSeparator();

    QAction *infoAction = new QAction(tr("Get Info"), this);
    infoAction->setShortcut(QKeySequence("Ctrl+I"));
    fileMenu->addAction(infoAction);
    m_getInfoAction = infoAction;
    connect(infoAction, &QAction::triggered, this, &FileManagerMainWindow::getInfo);

    // Add the File menu to the menu bar
    m_menuBar->addMenu(fileMenu);

    // Create the Edit menu
    QMenu *editMenu = new QMenu(tr("Edit"), this);

    // Add the usual menu items to the Edit menu
    editMenu->addAction(tr("Undo"));
    editMenu->actions().last()->setShortcut(QKeySequence("Ctrl+Z"));
    // Set disabled because we don't have an undo stack
    editMenu->actions().last()->setEnabled(false);
    editMenu->addSeparator();

    editMenu->addAction(tr("Cut"));
    editMenu->actions().last()->setShortcut(QKeySequence("Ctrl+X"));
    // Put the paths of the selected files on the clipboard so that we can use them when pasting
    connect(editMenu->actions().last(), &QAction::triggered, this, [this]() {
        // Get all selected indexes
        QModelIndexList selectedIndexes = m_iconView->selectionModel()->selectedIndexes();
        // Get the file paths of the selected indexes
        QStringList filePaths;
        for (const QModelIndex &index : selectedIndexes) {
            if (! filePaths.contains(m_fileSystemModel->filePath(m_proxyModel->mapToSource(index)))) {
                filePaths.append(m_fileSystemModel->filePath(m_proxyModel->mapToSource(index)));
            }
        }
        qDebug() << "Copying the following files to the clipboard:";
        for (const QString &filePath : filePaths) {
            qDebug() << filePath;
        }
        // Put the file paths on the clipboard using mimeData->setData("text/uri-list", ...)
        QMimeData *mimeData = new QMimeData;
        QList<QUrl> urls;
        for (const QString &filePath : filePaths) {
            urls.append(QUrl::fromLocalFile(filePath));
        }
        mimeData->setUrls(urls);
        QApplication::clipboard()->setMimeData(mimeData);
        // Set the "cut" property of the clipboard to true
        // so that we know that we are cutting and not copying
        QApplication::clipboard()->setProperty("cut", true);
    });

    editMenu->addAction(tr("Copy"));
    editMenu->actions().last()->setShortcut(QKeySequence("Ctrl+C"));
    // Put the paths of the selected files on the clipboard so that we can use them when pasting
    connect(editMenu->actions().last(), &QAction::triggered, this, [this]() {
        // Get all selected indexes
        QModelIndexList selectedIndexes = m_iconView->selectionModel()->selectedIndexes();
        // Get the file paths of the selected indexes
        QStringList filePaths;
        for (const QModelIndex &index : selectedIndexes) {
            if (! filePaths.contains(m_fileSystemModel->filePath(m_proxyModel->mapToSource(index)))) {
                filePaths.append(m_fileSystemModel->filePath(m_proxyModel->mapToSource(index)));
            }
        }
        qDebug() << "Copying the following files to the clipboard:";
        for (const QString &filePath : filePaths) {
            qDebug() << filePath;
        }
        // Put the file paths on the clipboard using mimeData->setData("text/uri-list", ...)
        QMimeData *mimeData = new QMimeData;
        QList<QUrl> urls;
        for (const QString &filePath : filePaths) {
            urls.append(QUrl::fromLocalFile(filePath));
        }
        mimeData->setUrls(urls);
        QApplication::clipboard()->setMimeData(mimeData);
        // Set the "cut" property of the clipboard to false
        // so that we know that we are copying and not cutting
        QApplication::clipboard()->setProperty("cut", false);
    });

    editMenu->addAction(tr("Paste"));
    editMenu->actions().last()->setShortcut(QKeySequence("Ctrl+V"));
    // Lambda that prints the paths of the files that are on the clipboard
    connect(editMenu->actions().last(), &QAction::triggered, this, [this]() {
        qDebug() << "Pasting the following files from the clipboard:";
        QClipboard *clipboard = QApplication::clipboard();

        // Check if we have URLs on the clipboard using mimeData
        if (clipboard->mimeData()->hasUrls()) {
            qDebug() << "The clipboard has URLs: " << clipboard->mimeData()->urls();
            // Check if they are all file://
            bool allFileUrls = true;
            for (const QUrl &url : clipboard->mimeData()->urls()) {
                if (!url.isLocalFile()) {
                    allFileUrls = false;
                    break;
                }
            }
            qDebug() << "All URLs are file://: " << allFileUrls;

            // Find out whether they were cut or copied
            bool filesWereCut = clipboard->property("cut").toBool();

            // If they were cut, move them
            if (filesWereCut) {
                // Construct the command line arguments from the URLs on the clipboard
                QClipboard *clipboard = QApplication::clipboard();
                QList<QUrl> urls = clipboard->mimeData()->urls();

                // Get the destination directory based on the root index of the current view
                QModelIndex rootIndex = m_treeView->rootIndex();
                // Map the root index to the source model
                rootIndex = m_proxyModel->mapToSource(rootIndex);
                QString destinationDirectory = m_fileSystemModel->filePath(rootIndex);
                QStringList sourceFilePaths;
                for (const QUrl &url : urls) {
                    sourceFilePaths.append(url.toLocalFile());
                    qDebug() << "Shall move " << url.toLocalFile() << " to " << destinationDirectory;
                }
                FileOperationManager::moveWithProgress(sourceFilePaths, destinationDirectory);
            }

            if (!filesWereCut) {
                // Construct the command line arguments from the URLs on the clipboard
                QClipboard *clipboard = QApplication::clipboard();
                QList<QUrl> urls = clipboard->mimeData()->urls();

                // Get the destination directory based on the root index of the current view
                QModelIndex rootIndex = m_treeView->rootIndex();
                // Map the root index to the source model
                rootIndex = m_proxyModel->mapToSource(rootIndex);
                QString destinationDirectory = m_fileSystemModel->filePath(rootIndex);
                QStringList sourceFilePaths;
                for (const QUrl &url : urls) {
                    sourceFilePaths.append(url.toLocalFile());
                    qDebug() << "Shall copy " << url.toLocalFile() << " to " << destinationDirectory;
                }
                FileOperationManager::copyWithProgress(sourceFilePaths, destinationDirectory);
           }
        }
    });


    editMenu->addAction(tr("Move to Trash"));
    editMenu->actions().last()->setShortcut(QKeySequence("Ctrl-Backspace"));
    connect(editMenu->actions().last(), &QAction::triggered, this, [this]() {
        // Get all selected indexes
        QModelIndexList selectedIndexes = m_iconView->selectionModel()->selectedIndexes();
        // Disregard all indexes that are not for the first column
        for (int i = 0; i < selectedIndexes.size(); i++) {
            if (selectedIndexes.at(i).column() != 0) {
                selectedIndexes.removeAt(i);
                i--;
            }
        }
        // Get the file paths of the selected indexes
        QStringList filePaths;
                for (const QModelIndex &index : selectedIndexes) {
            filePaths.append(m_fileSystemModel->filePath(m_proxyModel->mapToSource(index)));
        }
        qDebug() << "Moving to trash the following files:";
        for (const QString &filePath : filePaths) {
            qDebug() << filePath;
        }
        // Move the files to the trash
        TrashHandler trashHandler(this);
        trashHandler.moveToTrash(filePaths);

    });
    m_moveToTrashAction = editMenu->actions().last();
    m_moveToTrashAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Backspace));

    editMenu->addSeparator();
    QAction *selectAllAction = new QAction(tr("Select All"), this);
    selectAllAction->setShortcut(QKeySequence("Ctrl+A"));
    editMenu->addAction(selectAllAction);

    // Connect the triggered() signal of the Select All action to a slot
    connect(selectAllAction, &QAction::triggered, this, &FileManagerMainWindow::selectAll);

    // Create the Rename action
    m_renameAction =
            editMenu->addAction(tr("Rename..."), this, &FileManagerMainWindow::renameSelectedItem);

    m_renameAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
    editMenu->addAction(m_renameAction);

    editMenu->addSeparator();


    editMenu->addAction(tr("Preferences..."), this, &FileManagerMainWindow::showPreferencesDialog);

    editMenu->addSeparator();

    // Create the "Empty Trash" action
    m_emptyTrashAction = new QAction(tr("Empty Trash"), this);
    editMenu->addAction(m_emptyTrashAction);
    connect(m_emptyTrashAction, &QAction::triggered, this, [this]() {
        TrashHandler::emptyTrash();
    });
    // Whenever the Edit menu is about to be shown, check if the trash is empty
    // and enable/disable the "Empty Trash" action accordingly
    connect(editMenu, &QMenu::aboutToShow, this, [this, editMenu]() {
        updateEmptyTrashMenu();
    });

    // Add the Edit menu to the menu bar
    m_menuBar->addMenu(editMenu);

    // Create the View menu
    QMenu *viewMenu = new QMenu(tr("View"), this);

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

    // Create the Show/Hide Hidden Files action
    QAction *showHideHiddenFilesAction = new QAction(tr("Show Hidden Files"), this);
    m_showHiddenFilesAction = showHideHiddenFilesAction;
    viewMenu->addAction(showHideHiddenFilesAction);
    connect(showHideHiddenFilesAction, &QAction::triggered, this,
            &FileManagerMainWindow::showHideHiddenFiles);
    m_showHiddenFilesAction->setCheckable(true);
    viewMenu->addSeparator();

    // Create the Show/Hide Status Bar action
    QAction *showHideStatusBarAction = new QAction(tr("Show Status Bar"), this);
    m_showStatusBarAction = showHideStatusBarAction;
    viewMenu->addAction(showHideStatusBarAction);
    connect(showHideStatusBarAction, &QAction::triggered, this,
            &FileManagerMainWindow::showHideStatusBar);
    m_showStatusBarAction->setCheckable(true);
    m_menuBar->addMenu(viewMenu);

    connect(viewMenu, &QMenu::aboutToShow, this, [this, viewMenu]() {
        if (m_proxyModel->isFilteringEnabled()) {
            m_showHiddenFilesAction->setChecked(false);
        } else {
            m_showHiddenFilesAction->setChecked(true);
        }
        if (m_statusBar->isVisible()) {
            m_showStatusBarAction->setChecked(true);
        } else {
            m_showStatusBarAction->setChecked(false);
        }
    });

    // Create the Go menu
    QMenu *goMenu = new QMenu(tr("Go"), this);

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
        // Run the command "launch Zeroconf" and check its exit code
        if (QProcess::execute("launch Zeroconf") != 0) {
            QMessageBox::critical(this, tr("Error"), tr("Could not launch Zeroconf"));
            // NOTE: Implement the command "launch Zeroconf" like in helloSystem if you want to use this;
            // in the future we might think about implementing Zeroconf browsing here using
            // https://github.com/nitroshare/qmdnsengine
        }
    });

    QMenu *devicesMenu = new QMenu(tr("Devices"), this);
    goMenu->addMenu(devicesMenu);
    connect(devicesMenu, &QMenu::aboutToShow, this, [this, devicesMenu, goMenu]() {
        devicesMenu->clear();
        QDir mediaDir(AppGlobals::mediaPath);
        if (mediaDir.exists()) {
            for (const QString &entry : mediaDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
                QAction *action = devicesMenu->addAction(entry);
                connect(action, &QAction::triggered, this,
                        [this, entry]() { openFolderInNewWindow(AppGlobals::mediaPath +"/" + entry); });
            }
        }
    });

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

        // QDialog dialog(this); // Never do this
        QDialog* dialog = new QDialog(this); // Do this instead
        dialog->setWindowTitle(tr("Go to Folder..."));
        dialog->setWindowIcon(QIcon::fromTheme("folder"));
        dialog->setFixedWidth(400);
        QVBoxLayout *layout = new QVBoxLayout(dialog);
        QLabel *label = new QLabel(tr("Folder:"), dialog);
        layout->addWidget(label);
        QLineEdit *lineEdit = new QLineEdit(dialog);
        auto completer = new QCompleter(this);
        lineEdit->setCompleter(completer);
        layout->addWidget(lineEdit);
        completer->setCompletionMode(QCompleter::InlineCompletion);
        QFileSystemModel *fsModel = new QFileSystemModel(completer);
        fsModel->setFilter(QDir::Dirs|QDir::Drives|QDir::NoDotAndDotDot|QDir::AllDirs); // Only directories, no files
        completer->setModel(fsModel);
        // When the user enters "~" in the line edit, complete it to the home directory
        QObject::connect(lineEdit, &QLineEdit::textChanged, [=](const QString &text) {
            if (text.startsWith("~")) {
                QString completedText = QDir::homePath() + text.mid(1) + "/";
                lineEdit->setText(completedText);
                lineEdit->setCursorPosition(completedText.length()); // Place cursor at the end
            }
        });
        fsModel->setRootPath(QString());
        lineEdit->setCompleter(completer);
        lineEdit->setPlaceholderText(tr("Enter a folder path..."));
        lineEdit->setClearButtonEnabled(true);
        layout->addWidget(lineEdit);
        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog);
        layout->addWidget(buttonBox);
        connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
        if (dialog->exec() == QDialog::Accepted) {
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
    QMenu *helpMenu = new QMenu(tr("Help"), this);

    // Add an action to the Help menu
    QAction *a = helpMenu->addAction(tr("About The Filer"));

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
                QString filePath = m_fileSystemModel->filePath(m_proxyModel->mapToSource(index));
                open(filePath);
            },
            Qt::QueuedConnection);

    shortcut = new QShortcut(QKeySequence(Qt::ALT + Qt::CTRL + Qt::Key_Down), this);
    connect(
            shortcut, &QShortcut::activated, this,
            [this]() {
                QModelIndex index = m_treeView->currentIndex();
                QString filePath = m_fileSystemModel->filePath(m_proxyModel->mapToSource(index));
                openWith(filePath);
            },
            Qt::QueuedConnection);

    shortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Down), this);
    connect(
            shortcut, &QShortcut::activated, this,
            [this]() {
                QModelIndex index = m_treeView->currentIndex();
                QString filePath = m_fileSystemModel->filePath(m_proxyModel->mapToSource(index));
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
    aboutBox.setWindowTitle("About The Filer");
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
    QModelIndexList selectedIndexes = m_iconView->selectionModel()->selectedIndexes();

    // Disregard all indexes that are not for the first column
    for (int i = 0; i < selectedIndexes.size(); i++) {
        if (selectedIndexes.at(i).column() != 0) {
            selectedIndexes.removeAt(i);
            i--;
        }
    }

    qDebug() << "Selected indexes:" << selectedIndexes;

    // Calculate the size of the selected items on disk
    qint64 size = 0;
    for (const QModelIndex &index : selectedIndexes) {
        size += m_fileSystemModel->size(m_proxyModel->mapToSource(index));
    }

    // Format the size in a human-readable format using the user's locale settings
    QString sizeString = QLocale().formattedDataSize(size);

    // Show the number of selected items and their size on disk in the status bar
    m_statusBar->showMessage(
            QString("%1 items selected (%2)").arg(selectedIndexes.size()).arg(sizeString));

    // Print a message indicating that the function has completed
    qDebug() << "Completed" << Q_FUNC_INFO;
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
    qDebug() << Q_FUNC_INFO << rootPath;

    // Make path absolute and resolve symlinks
    QString resolvedRootPath = QFileInfo(rootPath).absoluteFilePath();
    if (QFileInfo(resolvedRootPath).isSymLink()) {
        resolvedRootPath = QFileInfo(resolvedRootPath).symLinkTarget();
    }

    // Check if the path exists, show an error dialog if it is not
    if (!QDir(resolvedRootPath).exists()) {
        QMessageBox::critical(nullptr, "Error", "This folder does not exist.");
        return;
    }

    // Check if the path exists and is a directory or a symlink to a directory, show an error dialog
    // if it is not
    qDebug() << "resolvedRootPath" << resolvedRootPath;
    if (!QFileInfo(resolvedRootPath).exists() || !QFileInfo(resolvedRootPath).isDir()) {
        QMessageBox::critical(nullptr, "Error", "This path is not a folder.");
        return;
    }

    // Check if the user has read and execute permissions for the path;
    // do nothing if the user does not have read and execute permissions
    if (!QFileInfo(resolvedRootPath).isReadable() || !QFileInfo(resolvedRootPath).isExecutable()) {
        return;
    }

    // Check if a window for the specified root path already exists
    bool windowExists = false;
    for (FileManagerMainWindow *window : instances()) {
        if (window->m_fileSystemModel->rootPath() == resolvedRootPath) {
            // A window for the specified root path already exists
            window->bringToFront();
            windowExists = true;
            break;
        }
    }

    if (!windowExists) {
        // No window for the specified root path exists, so create a new one
        // Not setting a parent, so that the window does not get destroyed when the parent gets
        // destroyed
        FileManagerMainWindow *newWindow = new FileManagerMainWindow(nullptr, resolvedRootPath);
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
                    // Mao the selected index to the source model
                    const QModelIndex sourceIndex = m_proxyModel->mapToSource(selectedIndex);
                    customDelegate->startAnimation(sourceIndex);
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
        // Check if it is a command line tool and show an error dialog if it is
        if (bundle.isCommandLineTool()) {
            QMessageBox::information(nullptr, 0, "This is a command line tool. You can run it "
                                                    "from the terminal.");
            // TODO: Run the command line tool in a terminal
            // For this, we need to get the terminal application to be used
            return;
        }
        // Use the "launch" command to open the file
        qDebug() << "Launching:" << filePath;
        QProcess process;
        process.setProgram("launch");
        process.setArguments({ filePath });
        process.startDetached();

        // Show the busy cursor
        QApplication::setOverrideCursor(Qt::BusyCursor);
        QTimer::singleShot(2000, this, []() {
            QApplication::restoreOverrideCursor();
        });

    } else {

        // Check if the filePath is a directory or a file
        if (QFileInfo(filePath).isDir()) {
            QString rootPath = m_fileSystemModel->filePath(m_proxyModel->mapToSource(m_selectionModel->currentIndex()));
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
                    window->bringToFront();
                } else {
                    // If we don't, open a new window
                    qDebug() << "Opening new window:" << filePath;
                    openFolderInNewWindow(filePath);
                }
            }
        } else {
            // Use the "open" command to open the file
            qDebug() << "Opening:" << filePath;
            QProcess process;
            process.setProgram("open");
            process.setArguments({ filePath });
            process.startDetached();

            // Show the busy cursor
            QApplication::setOverrideCursor(Qt::BusyCursor);
            QTimer::singleShot(2000, this, []() {
                QApplication::restoreOverrideCursor();
            });
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

    // Show the busy cursor
    QApplication::setOverrideCursor(Qt::BusyCursor);
    QTimer::singleShot(2000, this, []() {
        QApplication::restoreOverrideCursor();
    });
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

    // Check if the item to be renamed is a mountpoint

    const QString currentPath = m_proxyModel->mapToSource(selectedIndex).data(QFileSystemModel::FilePathRole).toString();
    QString absoluteFilePath = QFileInfo(currentPath).absoluteFilePath();
    // if absoluteFilePath is a symlink, resolve it
    if (QFileInfo(absoluteFilePath).isSymLink()) {
        absoluteFilePath = QFileInfo(absoluteFilePath).symLinkTarget();
    }
    qDebug() << "absoluteFilePath:" << absoluteFilePath;

    if (Mountpoints::isMountpoint(absoluteFilePath)) {
        // Get the filesystem type using QStorageInfo
        const QString filesystemType = QStorageInfo(absoluteFilePath).fileSystemType();
        qDebug() << "Filesystem type:" << filesystemType;
        QStringList renameableFilesystems = { "ext2", "ext3", "ext4", "reiserfs", "reiser4", "ufs", "vfat", "exfat", "ntfs" };
        // If the filesystem is not in the list of renameable filesystems, disable renaming
        if (!renameableFilesystems.contains(filesystemType)) {
            QMessageBox::information(this, tr("Rename"), tr("Renaming is not supported yet for the %1 filesystem type.").arg(filesystemType));
            return;
        }
    }

    // Construct a dialog using this QLineEdit
    // QDialog dialog(this); // Never do this
    QDialog* dialog = new QDialog(this); // Do this instead
    QLineEdit* lineEdit = new QLineEdit(dialog);
    lineEdit->setValidator(&validator);
    dialog->setWindowTitle(tr("Rename"));
    dialog->setLayout(new QVBoxLayout());
    dialog->layout()->addWidget(lineEdit);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, dialog);
    connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    dialog->layout()->addWidget(buttonBox);
    dialog->adjustSize();
    dialog->setFixedWidth(400);
    lineEdit->setText(currentName);
    // Select all but the extension
    lineEdit->setSelection(0, currentName.lastIndexOf('.'));
    lineEdit->setFocus();
    int result = dialog->exec();

    // Get the new name of the selected item
    const QString newName = lineEdit->text();

    if (result == QDialog::Accepted) {
        ok = true;
    } else {
        ok = false;
    }

    if (!ok || newName.isEmpty() || newName == currentName) {
        // The user canceled the dialog or didn't enter a new name
        return;
    }

    qDebug() << "newName:" << newName;
    qDebug() << "currentName:" << currentName;

    // If there was an extension, compare the old and the new extension
    // and if they are different, ask the user if they want to continue
    if (currentName.lastIndexOf('.') != -1) {
        if (currentName.right(currentName.length() - currentName.lastIndexOf('.') - 1) != newName.right(newName.length() - newName.lastIndexOf('.') - 1)) {
            // The extensions are different
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, 0, tr("Do you really want to change the file extension?"), QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::No) {
                return;
            }
        }
    }

    if (Mountpoints::isMountpoint(absoluteFilePath)) {

        // TODO: Possibly move everything in this if statement to a separate class,
        // similar to the FileOperationManager class

        // The item to be renamed is a mountpoint, so we need to run:
        // sudo -A -E renamedisk <old name> <new name>

        // Check if we are on FreeBSD and if we are not, show an error message
        if (QSysInfo::kernelType() != "freebsd") {
            QMessageBox::critical(0, "Filer", "The 'renamedisk' command has only been implemented for FreeBSD yet.");
            return;
        }

        QStringList renamediskBinaryCandidates;

        renamediskBinaryCandidates << QCoreApplication::applicationDirPath() + QString("/bin/renamedisk")
                                      << QCoreApplication::applicationDirPath() + QString("/../../Resources/renamedisk")
                                      << QCoreApplication::applicationDirPath() + QString("/../bin/renamedisk")
                                      << QCoreApplication::applicationDirPath() + QString("/renamedisk/renamedisk");

        QString foundBinary;

        for (const QString &renamediskBinaryCandidate : renamediskBinaryCandidates) {
            if (QFile::exists(renamediskBinaryCandidate) && QFileInfo(renamediskBinaryCandidate).isExecutable()) {
                foundBinary = renamediskBinaryCandidate;
                break;
            }
        }

        if (foundBinary.isEmpty()) {
            // Not found
            QMessageBox::critical(0, "Filer", "The 'renamedisk' command is missing. It should have been shipped with this application.");
            return;
        }

        QString oldName = currentPath.split("/").last();
        QProcess *p = new QProcess(this);
        // TODO: Check if we need sudo at all for this kind of filesystem; e.g., if it's a FAT32 filesystem
        // then we don't need sudo
        p->setProgram("sudo");
        p->setArguments({ "-A", "-E", foundBinary, absoluteFilePath, newName });
        qDebug() << p->program() << "'" + p->arguments().join("' '") + "'";
        p->start();
        p->waitForFinished();
        qDebug() << "renamedisk exit code:" << p->exitCode();
        if (p->exitCode() != 0) {
            QStringList errorLines = QString(p->readAllStandardError()).split("\n");
            for (const QString &errorLine : errorLines) {
                qCritical() << errorLine;
            }
            QMessageBox::critical(this, tr("Error"), tr("Could not rename %1 to %2").arg(oldName).arg(newName));
        } else {
            qDebug() << "Renamed" << currentPath << "to" << newName;
            // The view will automatically update itself; works
        }
        return;
    }

    // Rename the selected item in the file system
    const QString newPath = currentPath.left(currentPath.lastIndexOf("/") + 1) + newName;
    if(!QFile::rename(currentPath, newPath)) {
        QMessageBox::critical(this, tr("Error"), tr("Could not rename %1 to %2").arg(currentPath).arg(newPath));
    } else {
        qDebug() << "Renamed" << currentPath << "to" << newPath;
        // The view will automatically update itself; works
    }
}

void FileManagerMainWindow::updateMenus() {
    // Print the name of the called function
    qDebug() << Q_FUNC_INFO;

    // Check if the current directory is writable and enable/disable the 'New' action accordingly
    bool hasWritePermissions = QFileInfo(m_currentDir).isWritable();
    m_newAction->setEnabled(hasWritePermissions);

    // Get the list of selected items
    const QModelIndexList selectedIndexes = m_selectionModel->selectedIndexes();

    // Check if there is exactly one selected item
    if (selectedIndexes.size() == 1) {
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
        bool allSelectedItemsCanShowContents = true;
        for (const QModelIndex &index: selectedIndexes) {
            // Check if the selected item can show its contents
            QString filePath = m_fileSystemModel->filePath(m_proxyModel->mapToSource(index));
            ApplicationBundle *bundle = new ApplicationBundle(filePath);
            if (bundle->isValid() && bundle->type() != ApplicationBundle::Type::DesktopFile) {
                // Do nothing in this case
            } else {
                allSelectedItemsCanShowContents = false;
            }
            delete bundle;
        }
        if (allSelectedItemsCanShowContents) {
            m_showContentsAction->setEnabled(true);
        } else {
            m_showContentsAction->setEnabled(false);
        }
    }

    // Disable the Move to Trash action if the selected item is already in the trash
    // or it is a symlink to the Trash folder
    QStringList filePaths;
    for (const QModelIndex &index: selectedIndexes) {
        filePaths.append(m_fileSystemModel->filePath(m_proxyModel->mapToSource(index)));
    }
    for (const QString &filePath: filePaths) {
        QString resolvedFilePath = filePath;
        // TODO: Remove the following if statement once we no longer use symlinks to the Trash folder
        if (QFileInfo(filePath).isSymLink()) {
            QString linkTarget = QFileInfo(filePath).symLinkTarget();
            if (!linkTarget.isEmpty()) {
                QString parentPath = QFileInfo(filePath).dir().absolutePath();
                if (parentPath == QDir::homePath() + "/Desktop") {
                    resolvedFilePath = linkTarget;
                }
            }
        }

        if (resolvedFilePath.startsWith(TrashHandler::getTrashPath())) {
            m_moveToTrashAction->setEnabled(false);
        }
    }
    updateEmptyTrashMenu();
}

void FileManagerMainWindow::updateEmptyTrashMenu() {
    qDebug() << Q_FUNC_INFO;
    // Disable the Empty Trash action if the trash is already empty
    if (TrashHandler::isEmpty()) {
        qDebug() << "Trash is empty, disabling Empty Trash action";
        m_emptyTrashAction->setEnabled(false);
    } else {
        qDebug() << "Trash is not empty, enabling Empty Trash action";
        m_emptyTrashAction->setEnabled(true);
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

QAbstractItemView* FileManagerMainWindow::getCurrentView() const
{
    QAbstractItemView* currentActiveView = nullptr;

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

// Method that takes a QStringList of paths and selects the items in the view;
// scroll to the first item in the list
void FileManagerMainWindow::selectItems(const QStringList &paths)
{
    // Print the name of the called function
    qDebug() << Q_FUNC_INFO;

    bringToFront();

    // Get the current view
    QAbstractItemView* currentActiveView = getCurrentView();

    // Iterate over the list of paths
    for (const QString& path : paths) {
        // Get the index of the item with the given path
        qDebug("Path: %s", path.toStdString().c_str());
        const QModelIndex index = m_fileSystemModel->index(path);
        // Map the index to the proxy model
        const QModelIndex proxyIndex = m_proxyModel->mapFromSource(index);
        // Check if the index is valid
        if (!proxyIndex.isValid()) {
            // The index is invalid, so skip it
            qDebug() << "Skipping invalid proxyIndex" << proxyIndex;
            continue;
        }
        qDebug() << "Selecting item with path" << path << "and proxyIndex" << proxyIndex;
        // Unselect all items
        m_selectionModel->clearSelection();
        // Select the item
        m_selectionModel->select(proxyIndex, QItemSelectionModel::Select);

        // Wait until the view is not busy
        while (currentActiveView->property("isBusy").toBool()) {
            qDebug() << "Waiting for view to be ready";
            QCoreApplication::processEvents();
        }

        // Scroll to the item based on the current active view
        currentActiveView->scrollTo(proxyIndex, QAbstractItemView::PositionAtCenter);
    }

}

void FileManagerMainWindow::bringToFront()
{
        raise(); // Bring the window to the front
        activateWindow(); // Activate the window
        // If it is minimized, restore it
        if (isMinimized()) {
            showNormal(); // Show the window normally (unminimize)
        }
};

void FileManagerMainWindow::getInfo() {
    // Get all selected items
    QModelIndexList selectedIndexes = getCurrentView()->selectionModel()->selectedIndexes();

    // If no items are selected, show the info for the current directory
    if (selectedIndexes.isEmpty()) {
        // Get the absolute path of the current directory
        QString filePath = m_currentDir;
        // Destroy the dialog when it is closed
        InfoDialog *infoDialog = InfoDialog::getInstance(filePath, this);
        infoDialog->setAttribute(Qt::WA_DeleteOnClose);
        infoDialog->show();
    } else {
        // Iterate over the selected items

        for (QModelIndex index: selectedIndexes) {
            // Get the absolute path of the item represented by the index, using the model
            QString filePath = m_fileSystemModel->data(m_proxyModel->mapToSource(index),
                                                       QFileSystemModel::FilePathRole).toString();
            qDebug() << "XXXXXXXXXXXXXX Selected file path:" << filePath;
            // Destroy the dialog when it is closed
            InfoDialog *infoDialog = InfoDialog::getInstance(filePath, this);
            infoDialog->setAttribute(Qt::WA_DeleteOnClose);
            infoDialog->show();
        }
    }
}

/*
void FileManagerMainWindow::setFilterRegExpForHiddenFiles(QSortFilterProxyModel *proxyModel, const QString &hiddenFilePath)
{
    QStringList hiddenFiles;
    QFile hiddenFile(hiddenFilePath);

    // Check if the hidden file exists and open it for reading
    if (hiddenFile.exists()) {
        qDebug() << "xxxxxxxxxxx Hidden file exists: " << hiddenFilePath;
        if (hiddenFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&hiddenFile);
            while (!in.atEnd())
            {
                QString line = in.readLine().trimmed();
                if (!line.isEmpty())
                    hiddenFiles.append(QRegExp::escape(line));
            }
            hiddenFile.close();
        }
    }

    if (!hiddenFiles.isEmpty())
    {
        // Construct a regular expression pattern that filters out filenames
        // based on the contents of the hiddenFiles list and hides files starting with a dot.
        // The pattern uses negative lookahead assertions to match filenames that should be hidden.
        // - `^(?: ... )*$`: Anchors the pattern to the start and end of the filename and matches the entire filename.
        // - `(?:(?! ... ).)*`: Uses negative lookahead to assert that the filename is not followed by any of the specified patterns or a dot (hiddenFiles.join('|') and \\.) at any position.
        // - `hiddenFiles.join('|')`: Joins the hidden filenames with the OR operator (|) to create a pattern that matches any of the hidden filenames.
        // - `\\.`: Matches a literal dot character to account for filenames starting with a dot.
        // The resulting pattern hides files matching hidden filenames or starting with a dot, while showing other files.
        QString pattern = "^(?:(?!" + hiddenFiles.join('|') + "|\\.).)*$";
        QRegExp regExp(pattern);
        qDebug() << "xxxxxxxxxx Filtering out hidden files using pattern:" << pattern;
        proxyModel->setFilterRegExp(regExp);

    }
    else
    {
        proxyModel->setFilterRegExp(QRegExp()); // Reset filter
        QString dirName = QFileInfo(hiddenFilePath).dir().absolutePath();
        // Hide all files starting with a dot, but not the directory being shown itself (dirName)
        proxyModel->setFilterRegExp(QRegExp("^(?:(?!\\." + dirName + ").)*$"));
    }
}
*/

void FileManagerMainWindow::handleSelectionChange()
{
    updateStatusBar();
    updateMenus();
}

void FileManagerMainWindow::showPreferencesDialog()
{
    PreferencesDialog *preferencesDialog = PreferencesDialog::getInstance();
    preferencesDialog->show();
    // Connect the dialog's prefsChanged signal to the slot that updates the view
    connect(preferencesDialog, &PreferencesDialog::prefsChanged, this, &FileManagerMainWindow::refresh);
}

void FileManagerMainWindow::handleScreenChange(const QRect &geometry) {
    qDebug() << "Screen changed:"
             << "Geometry:" << geometry;

    // We are doing all the work only in the first instance (also for the other windows), so that it is done only once
    if (m_isFirstInstance) {

        // Move the first instance to the main screen and resize it to the screen size
        // Move to QApplication::primaryScreen()
        move(QApplication::desktop()->screenGeometry(0).topLeft());

        setFixedSize(QApplication::desktop()->screenGeometry(0).size());
        refresh();

        // Close all desktop picture windows and create new ones
        QList<QWidget*> topLevelWidgets = QApplication::topLevelWidgets();
        qDebug() << "Top level widgets:" << topLevelWidgets;
        for (QWidget *widget : topLevelWidgets) {
            qDebug() << widget->objectName();
            if (widget->objectName() == AppGlobals::desktopPictureWindowObjectName) {
                qDebug() << widget->objectName() << "is a desktop picture window, closing it";
                widget->close();
            }
        }
        displayPicturesOnAllScreens();
    }
}

// Opens windows that do nothing but show the desktop pictures on all screens but the main one
void FileManagerMainWindow::displayPicturesOnAllScreens() {
    QString desktopPicturePath = QSettings().value("desktopPicture", "/usr/local/share/slim/themes/default/background.jpg").toString();

    if (!QFileInfo(desktopPicturePath).exists()) {
        return;
    }

    QApplication &app = *static_cast<QApplication*>(QApplication::instance());
    QList<QScreen*> screens = app.screens();

    for (QScreen *screen : screens) {

        // Skip the screen that the main window is on
        if (screen == QApplication::primaryScreen()) {
            continue;
        }

        QRect screenGeometry = screen->geometry();

        QPixmap desktopPixmap = QPixmap(desktopPicturePath);
        QLabel *label = new QLabel;
        label->setPixmap(desktopPixmap.scaled(screenGeometry.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));

        QWidget *window = new QWidget;
        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(label);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        window->setContentsMargins(0, 0, 0, 0);
        window->setLayout(layout);
        window->setGeometry(screenGeometry);
        window->setObjectName(AppGlobals::desktopPictureWindowObjectName);

        window->setFixedSize(screenGeometry.size());
        window->setAttribute(Qt::WA_X11NetWmWindowTypeDesktop, true);
        window->setAttribute(Qt::WA_X11DoNotAcceptFocus, true);
        window->setWindowFlags(Qt::FramelessWindowHint);
        window->setWindowFlags(Qt::Tool);
        window->show();
    }
}

void FileManagerMainWindow::showHideHiddenFiles() {
    // Check if the menu action is checked or not
    if (m_proxyModel->isFilteringEnabled()) {
        qDebug() << "showHideHiddenFiles(): Hiding hidden files";
        m_proxyModel->setFilteringEnabled(false);
    }
    else {
        qDebug() << "showHideHiddenFiles(): Showing hidden files";
        m_proxyModel->setFilteringEnabled(true);
    }
}
