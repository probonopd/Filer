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

#include "applicationbundle.h"
#include "customitemdelegate.h"
#include "mainwindow.h"
#include "customfileiconprovider.h"

#include <QFileIconProvider>
#include <QFileInfo>
#include <QIcon>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QTimer>
#include <Qt>
#include <QDebug>
#include <QMoveEvent>
#include <QTreeWidgetItem>


// Constructor that takes a QObject pointer and a QFileSystemModel pointer as arguments
CustomItemDelegate::CustomItemDelegate(QObject *parent, QFileSystemModel *fileSystemModel)
    : QStyledItemDelegate(parent), m_fileSystemModel(fileSystemModel)
{
    // Initialize the m_fileSystemModel member variable with the provided QFileSystemModel pointer
    m_fileSystemModel = fileSystemModel;


    // Create an instance of the custom icon provider
    CustomFileIconProvider *iconProvider = new CustomFileIconProvider();

    // Set the icon provider for the model
    m_fileSystemModel->setIconProvider(iconProvider);
}

// Implement the destructor of the CustomItemDelegate class
CustomItemDelegate::~CustomItemDelegate()
{
    // The destructor does not have any additional functionality
}

// When a delegate object is moved, save its coordinates relative to the parent widget
// Implement the eventFilter() method in the CustomItemDelegate class
bool CustomItemDelegate::eventFilter(QObject *object, QEvent *event)
{

    qDebug() << "Received event:" << event->type();

    // Check if the event is a move event
    if (event->type() == QEvent::Move) {
        qDebug() << "Move event";

        // Check if the object is a QWidget
        if (QWidget *widget = qobject_cast<QWidget*>(object)) {
            // Get the global position of the widget in the view
            QPoint globalPos = widget->mapToGlobal(QPoint(0, 0));

            // Get the coordinates of the object relative to the parent widget
            int x = globalPos.x();
            int y = globalPos.y();
            qDebug() << "x: " << x << "y: " << y;

            qDebug() << "TODO: Save the coordinates of the delegate object relative to the parent widget";

            // Return true to indicate that the event was handled
            return true;
        }
    }

    // Return false to indicate that the event was not handled
    return false;
}

// Reimplement the displayText() function of the CustomItemDelegate class
// to show a different name for application bundles
QString CustomItemDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    // Get the original name of the item
    QString originalName = value.toString();

    // QString originalName = QStyledItemDelegate::displayText(value, locale);
    if(originalName.endsWith(".app") || originalName.endsWith(".AppDir") || originalName.endsWith(".AppImage") || originalName.endsWith(".desktop")) {
        // Return the default name
        return QFileInfo(originalName).completeBaseName();
    } else {
        // Return the default name
        return QStyledItemDelegate::displayText(value, locale);
    }
}

// Reimplement the paint() function of the CustomItemDelegate class
void CustomItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{

    /* To persist the fact that QModelItemDelegate objects in a QIconView have been moved,
     * we need to update the model to reflect the new positions of the delegate.
     * In Qt, the model is responsible for storing the data that is displayed in a view.
     * This means that if we want to persist changes to the positions of QModelItemDelegate objects in a QIconView,
     * we need to update the model to reflect the new positions of the delegate.
     */

    // Get the model that emitted the paint() signal
    QObject *sender = QObject::sender();

    // Cast the sender to a QFileSystemModel
    QFileSystemModel *model = this->m_fileSystemModel;

    // Get the current position of the delegate in the view
    QRect rect = option.rect;

/*
    // Save the rect of the delegate in the model and check if it was successful
    if (model->setData(index, rect, Qt::UserRole + 1)) {
        // The rect was saved successfully
        qDebug() << "Saved rect of delegate in model";
    } else {
        // The rect was not saved successfully
        qDebug() << "Failed to save rect of delegate in model";
    }
    model->setData(index, rect, Qt::UserRole);
*/

    // An option is a set of parameters that is passed to a style to draw a primitive element;
    // we are customizing the appearance of the delegate before it is drawn
    QStyleOptionViewItem customizedOption = option;

    // Find out whether the instance is the first instance (the Desktop)
    bool isFirstInstance = static_cast<FileManagerMainWindow*>(parent())->m_is_first_instance;

    // Set the color of the text in the option to white if the desktop is drawn
    if(isFirstInstance) {
        customizedOption.palette.setColor(QPalette::Text, Qt::white);
    }

    // Get the file path of the item
    QString filePath = static_cast<FileManagerMainWindow*>(parent())->m_fileSystemModel->filePath(index);

    // Create a QFileInfo object for the item
    QFileInfo fileInfo(filePath);

    // Get the original name of the item
    QString originalName = fileInfo.fileName();

    // Set the font of the text to italic for symlinks
    if (fileInfo.isSymLink()) {
        customizedOption.font.setItalic(true);
    }

    // Call the base class implementation of the paint() function
    // and make it use our customized option
    QStyledItemDelegate::paint(painter, customizedOption, index);
    return;
/*
 *
 *  The alternative would be to customize the drawing of the delegate... but in this case it looks
 *  like we need to do everything ourselves, including drawing the icon, the text, etc...
 *  So, for now, we will stick with the approach of customizing the option and then calling the
 *  base class implementation of the paint() function.

    // Get the file path of the item
    QString filePath = static_cast<FileManagerMainWindow*>(parent())->m_fileSystemModel->filePath(index);

    // Find out the class of the central widget
    bool isListView = false;
    QString className = static_cast<FileManagerMainWindow*>(parent())->centralWidget()->metaObject()->className();
    if(className == "QListView")
        isListView = true;
    qDebug() << "isListView:" << isListView;

    qDebug() << "Painting delegate for" << filePath << "at x: " << rect.x() << "y: " << rect.y();

    // Create a QFileInfo object for the item
    QFileInfo fileInfo(filePath);

    // Get the original name of the item
    QString originalName = fileInfo.fileName();

    // Create a QFileIconProvider object
    QFileIconProvider iconProvider;

    // Get the icon for the file info's MIME type
    QIcon icon = iconProvider.icon(fileInfo);

    // Get the model item flags for the item
    Qt::ItemFlags flags = model->flags(index);

    // Check if the original name ends with ".app", ".AppDir", or ".AppImage"
    if (originalName.endsWith(".app") || originalName.endsWith(".AppDir") || originalName.endsWith(".AppImage")) {

        // Set the icon to an application icon
        ApplicationBundle *bundle = new ApplicationBundle(filePath);
        icon = bundle->icon();

        // If the view is a QTreeView, prevent bundles from being expandable
        // Get the QTreeViewItem object if the view is a QTreeView
        QTreeWidgetItem *treeWidgetItem = nullptr;
        if(!isListView) {
            treeWidgetItem = static_cast<QTreeWidgetItem*>(index.internalPointer());
            qDebug() << "TODO: Prevent treeWidgetItem from being expandable:" << treeWidgetItem;
            // TODO: Find a way to do this without crashing
        }

    }

    // Get the icon size from the option object
    QSize iconSize = option.decorationSize; // E.g., QSize(32, 32)


    if(isListView) {
        // Calculate icon x position
        int iconX = option.rect.x() + (option.rect.width() - iconSize.width()) / 2;

        // Draw the application icon next to the item using the icon size
        icon.paint(painter, iconX, option.rect.y(), iconSize.width(), iconSize.height(), Qt::AlignTop | Qt::AlignHCenter, QIcon::Normal, QIcon::On);
    } else {
        // Icon only in the first column

        if (index.column() == 0) {
            // Draw the application icon next to the item using the icon size
            painter->save();
            // Move Painter to the right by 1/2 icon sizes
            painter->translate(iconSize.width() / 2, 0);
            icon.paint(painter, option.rect.x(), option.rect.y(), iconSize.width(), iconSize.height(), Qt::AlignTop | Qt::AlignHCenter, QIcon::Normal, QIcon::On);
            painter->restore();
        }
    }

    // Set the font of the text to italic for symlinks
    QFont font = painter->font();
    if(fileInfo.isSymbolicLink()){
        font.setItalic(true);
        painter->setFont(font);
    } else {
        font.setItalic(false);
        painter->setFont(font);
    }

    // Get the data for the item being displayed
    QVariant data = index.data();

    // Get the text to be displayed for the item
    QString text = displayText(data, QLocale::system());

    // Align the text to the center of the bounding rectangle
    QRect boundingRect;

    if(isListView) {
        // If it is the first instance (the Desktop), use black text for the shadow
        if(isFirstInstance) {
            // Draw drop shadow under the text
            painter->setPen(Qt::black);
            QRect shadowRect = option.rect.translated(1, 1);
            painter->drawText(shadowRect, Qt::AlignBottom | Qt::AlignHCenter, text, &shadowRect);
            painter->drawText(option.rect, Qt::AlignBottom | Qt::AlignHCenter, text, &boundingRect);
        }
        // If it is the first instance (the Desktop), use white text
        if(isFirstInstance) {
            painter->setPen(Qt::white);
        } else {
            painter->setPen(Qt::black);
        }
        painter->drawText(option.rect, Qt::AlignBottom | Qt::AlignHCenter, text, &boundingRect);
    } else {
        if(index.column() == 0) {
            painter->save();
            // Move Painter to the right by 2 icon sizes
            painter->translate(iconSize.width() * 2, 0);
            painter->drawText(option.rect, Qt::AlignVCenter | Qt::AlignLeft, text, &boundingRect);
            painter->restore();
        } else {
            painter->drawText(option.rect, Qt::AlignVCenter | Qt::AlignLeft, text, &boundingRect);
        }
    }
*/

/*
 * FIXME:
 * The issue here is that this method needs to be const, otherwise it does not get used.
 * But when it is const, then it cannot set iconShown, iconVisible, etc.
 *
    // Flash if the item was double-clicked
    if (option.state & QStyle::State_Selected && option.state & QStyle::State_MouseOver) {
        // Create a timer to flash the icon
        QTimer* timer = new QTimer(const_cast<CustomItemDelegate*>(this));
        connect(timer, &QTimer::timeout, [=]() {
            // Show or hide the icon on each timer tick
            if (iconShown) {
                iconShown = false;
                iconVisible = false;
            } else {
                iconShown = true;
                iconVisible = !iconVisible;
            }

            // Update the item to redraw the icon
            // Emit the dataChanged() signal from the item model
            m_fileSystemModel->dataChanged(index, index, {Qt::DecorationRole});

            // Stop the timer after 3 flashes
            if (flashCount >= 3) {
                timer->stop();
            } else {
                flashCount++;
            }
        });

        // Start the timer with a 100ms interval
        timer->start(100);
    }

*/
}

// Override the editorEvent() function to handle mouse events
// in order to show the right-click menu
bool CustomItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                                      const QModelIndex &index) {
    // Check if the event is a right-click
    if (event->type() == QEvent::MouseButtonPress && static_cast<QMouseEvent *>(event)->button() == Qt::RightButton) {


        // Get the absolute path of the item represented by the index, using the model
        QString filePath = model->data(index, QFileSystemModel::FilePathRole).toString();
        qDebug() << "Right-clicked on:" << filePath;

        // Create a context menu with the given actions
        QMenu menu;

        FileManagerMainWindow *mainWindow = static_cast<FileManagerMainWindow*>(parent());

        // Open
        QAction *openAction = new QAction("Open", &menu);
        openAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
        connect(openAction, &QAction::triggered, [=]() {
            // Open the file or folder using the mainwindow.h open() method
            mainWindow->open(filePath);
        });
        menu.addAction(openAction);

        QAction *openWithAction = new QAction(tr("Open with..."), this);
        openWithAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_O));
        menu.addAction(openWithAction);

        menu.addSeparator();

        QAction *showContentsAction = new QAction(tr("Show Contents"), this);
        showContentsAction->setEnabled(false);
        menu.addAction(showContentsAction);

        menu.addSeparator();

        QAction *getInfoAction = new QAction(tr("Get Info"), this);
        getInfoAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));
        getInfoAction->setEnabled(false);
        menu.addAction(getInfoAction);

        menu.addSeparator();

        QAction *renameAction = new QAction(tr("Rename..."), this);
        connect(renameAction, &QAction::triggered, [=]() {
            mainWindow->renameSelectedItem();
        });
        menu.addAction(renameAction);

        QAction *duplicateAction = new QAction(tr("Duplicate"), this);
        duplicateAction->setEnabled(false);
        menu.addAction(duplicateAction);

        QAction *moveToTrashAction = new QAction(tr("Move to Trash"), this);
        moveToTrashAction->setEnabled(false);
        menu.addAction(moveToTrashAction);

        menu.addSeparator();

        QAction *compressAction = new QAction(tr("Compress"), this);
        compressAction->setEnabled(false);
        menu.addAction(compressAction);

        // Show the context menu at the cursor position
        menu.exec(QCursor::pos());
        return true;
    }

    return false;
}