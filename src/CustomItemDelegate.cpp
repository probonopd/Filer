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

#include "ApplicationBundle.h"
#include "CustomItemDelegate.h"
#include "FileManagerMainWindow.h"

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
#include "ExtendedAttributes.h"
#include <QSize>
#include "ApplicationBundle.h"
#include "TrashHandler.h"
#include "InfoDialog.h"

// Constructor that takes a QObject pointer and a QFileSystemModel pointer as arguments
CustomItemDelegate::CustomItemDelegate(QObject* parent, CustomFileSystemModel* fileSystemModel)
        : QStyledItemDelegate(parent), m_fileSystemModel(fileSystemModel)
{

    // Initialize the m_fileSystemModel member variable with the provided QFileSystemModel pointer
    m_fileSystemModel = fileSystemModel;

    // Create an instance of the custom icon provider
    iconProvider = new CustomFileIconProvider();

    // Set the custom file system model in the file icon provider.
    iconProvider->setModel(m_fileSystemModel);

    // Set the icon provider for the model
    m_fileSystemModel->setIconProvider(iconProvider);

    // Create a QTimeLine instance for the animation
    animationTimeline = new QTimeLine(1000, this); // 1000 ms duration for the animation
    // Get faster towards the end of the animation
    animationTimeline->setEasingCurve(QEasingCurve::OutCubic);
    animationTimeline->setUpdateInterval(16); // About 60 FPS update rate
    // Verify the signal-slot connection for animation updates
    bool isConnected = connect(animationTimeline, &QTimeLine::valueChanged, this, &CustomItemDelegate::animationValueChanged);
    if (!isConnected) {
        qDebug() << "CustomItemDelegate::CustomItemDelegate: Could not connect animationTimeline to animationValueChanged";
    }

    connect(animationTimeline, &QTimeLine::finished, this, &CustomItemDelegate::animationFinished);

}

// Memory management rule of thumb for Qt:
// Try using a parent whenever using new.
// This way, the parent will take care of deleting the child.
// When this is not possible, then we need to declare the instance
// created with new as a member variable of the class and delete it
// in the destructor of the class.
// QUESTION: Which book teaches this?
CustomItemDelegate::~CustomItemDelegate()
{
    delete animationTimeline;
    delete iconProvider;
}

// Reimplement the displayText() function of the CustomItemDelegate class
// to show a different name for application bundles
QString CustomItemDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    // Get the original name of the item
    QString originalName = value.toString();

    // QString originalName = QStyledItemDelegate::displayText(value, locale);
    if (originalName.endsWith(".app") || originalName.endsWith(".AppDir")
        || originalName.endsWith(".AppImage") || originalName.endsWith(".desktop")) {
        // Return the default name
        return QFileInfo(originalName).completeBaseName();
    } else {
        // Return the default name
        return QStyledItemDelegate::displayText(value, locale);
    }
}

// Reimplement the paint() function of the CustomItemDelegate class
void CustomItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{

    /* To persist the fact that QModelItemDelegate objects in a QIconView have been moved,
     * we need to update the model to reflect the new positions of the delegate.
     * In Qt, the model is responsible for storing the data that is displayed in a view.
     * This means that if we want to persist changes to the positions of QModelItemDelegate objects
     * in a QIconView, we need to update the model to reflect the new positions of the delegate.
     */

    // Find out whether we are drawing for the first instane (desktop)
    // or for another instance (file manager window)
    QAbstractItemView *view = static_cast<QAbstractItemView *>(parent());
    // Get the parent (MainWindow) of the parent (QStackedWidget) of the view (CustomListView/QTreeView)
    FileManagerMainWindow *mainWindow = static_cast<FileManagerMainWindow *>(view->parent()->parent());
    // Assert that the parent of the parent of the view is a FileManagerMainWindow
    Q_ASSERT(mainWindow);

    bool isTreeView =  mainWindow->getCurrentView()->metaObject()->className() == QString("QTreeView");

    // Check if it is the first instance
    bool isFirstInstance = mainWindow->isFirstInstance();

    // Cast the sender to a QFileSystemModel
    QFileSystemModel *model = this->m_fileSystemModel;

    // Get the current position of the delegate in the view
    QRect rect = option.rect;

    // An option is a set of parameters that is passed to a style to draw a primitive element;
    // we are customizing the appearance of the delegate before it is drawn
    QStyleOptionViewItem customizedOption = option;

    m_currentIndex = index;
    m_currentOption = customizedOption;

    // For the desktop
    if (isFirstInstance) {
        // White text for the desktop
        customizedOption.palette.setColor(QPalette::Text, Qt::white);
        // Bold font for the desktop
        customizedOption.font.setBold(true);
    } else {
        customizedOption.palette.setColor(QPalette::Text, Qt::black);
    }

    // Get the file path of the item
    QString filePath = index.data(Qt::UserRole + 1).toString();

    // Create a QFileInfo object for the item
    QFileInfo fileInfo(filePath);

    // Get the original name of the item
    QString originalName = fileInfo.fileName();

    // Set the font of the text to italic for symlinks
    if (fileInfo.isSymLink()) {
        customizedOption.font.setItalic(true);
    }

    // Opened folders are drawn differently
    if (QFileInfo(filePath).isDir()) {
        // Check if we have a window open for the directory
        bool isOpen = static_cast<FileManagerMainWindow *>(parent())->instanceExists(filePath);
        if (isOpen) {
            // If it is already open, set the option to draw the icon as disabled
            customizedOption.state &= ~QStyle::State_Enabled;
            // Set opacity to 50% for the painter
            painter->setOpacity(0.5);
        } else {
            painter->setOpacity(1.0);
        }  
    }

    // Custom icon positions
    // Customize the icon position to reflect custom icon positions
    // Maybe it is wrong to do this here. Maybe the icon position should be managed in the view.
/*
    // Get the coordinates of the icon from the model
    QPoint customCoordinates = m_fileSystemModel->getIconCoordinates(fileInfo);
    // Only if the coordinates are valid, set them in the option that the superclass will use for drawing
    if (customCoordinates != QPoint(-1, -1)) {
        customizedOption.rect.setX(customCoordinates.x());
        customizedOption.rect.setY(customCoordinates.y());
        // We need to make the view aware of the new position of the delegate. Just because we are painting to a
        // different position, it doesn't mean that the view will be aware of it. We need to update the model to
        // reflect the new positions of the delegate.
        // So, how do we do that? We need to tell the view or the model the coordinates of the delegate.
        // Somehow on the CustomListView, setPositionForIndex(customCoordinates, index);
        // ???
    }
*/

    // Check if the current item is the one being animated
    bool isAnimatingItem = (index == m_animatedIndex);
    // Print its path if it is
    if (isAnimatingItem) {
        qDebug() << "::paint() - Animated item: " << filePath;
    }

    if (!isTreeView && isAnimatingItem && animationTimeline->state() == QTimeLine::Running)
    {
            // An option is a set of parameters that is passed to a style to draw a primitive element;
            // we are customizing the appearance of the delegate before it is drawn
            QStyleOptionViewItem customizedOptionForAnim = customizedOption;

        // Save state of the painter so that we can reset to it
        painter->save();

        // Retrieve the data from the model using the 'index'
        QVariant data = index.model()->data(index, Qt::DecorationRole);
        // Get the icon size from the view
        QSize iconSize = customizedOptionForAnim.decorationSize;
        QPixmap originalIcon = data.value<QIcon>().pixmap(QSize(iconSize)); // Adjust the icon size as needed; FIXME: Depends on the view

        // Calculate the scale factor for the icon
        qreal scaleFactor = 1.0 + 8.0 * currentAnimationValue;
        // qDebug() << "::paint() - Scale factor: " << scaleFactor;

        // Calculate the opacity (1.0 to 0.0)
        qreal opacity = 1.0 - currentAnimationValue;
        // qDebug() << "::paint() - Opacity: " << opacity;

        // Create a new scaled icon based on the scaleFactor
        QPixmap scaledIcon = originalIcon.scaled(originalIcon.width() * scaleFactor,
                                                 originalIcon.height() * scaleFactor,
                                                 Qt::KeepAspectRatio,
                                                 Qt::SmoothTransformation);

        // The rect also needs to be scaled
        customizedOptionForAnim.rect.setWidth(scaledIcon.width());
        customizedOptionForAnim.rect.setHeight(scaledIcon.height());

        // The rect needs to be moved to keep the icon centered
        customizedOptionForAnim.rect.moveCenter(rect.center());

        // Set the painter's opacity based on the animation value
        painter->setOpacity(opacity);

        // Draw the scaled and faded icon
        painter->drawPixmap(customizedOptionForAnim.rect, scaledIcon);

        // Restore the painter's state
        painter->restore();
    }

    // Call the superclass implementation of the paint() function
    QStyledItemDelegate::paint(painter, customizedOption, index);
    
}

// Override the editorEvent() function to handle mouse events
// in order to show the right-click menu
bool CustomItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                     const QStyleOptionViewItem &option, const QModelIndex &index)
{
    // Check if the event is a right-click
    if (event->type() == QEvent::MouseButtonPress
        && static_cast<QMouseEvent *>(event)->button() == Qt::RightButton) {

        // Get the absolute path of the item represented by the index, using the model
        QString filePath = model->data(index, QFileSystemModel::FilePathRole).toString();
        qDebug() << "Right-clicked on:" << filePath;

        // Create a context menu with the given actions
        QMenu menu;

        // From parent, go up until we have a FileManagerMainWindow
        // it could be the parent, the parent of the parent, etc.
        FileManagerMainWindow *mainWindow;
        auto *parent = this->parent();
        while (parent) {
            mainWindow = qobject_cast<FileManagerMainWindow *>(parent);
            if (mainWindow) {
                break;
            }
            parent = parent->parent();
        }

        // Set the item that was clicked on selected
        mainWindow->m_selectionModel->setCurrentIndex(index, QItemSelectionModel::Select);

        // Open
        QAction *openAction = new QAction("Open", &menu);
        openAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
        connect(openAction, &QAction::triggered, [=]() { 
            mainWindow->open(filePath); 
        });
        menu.addAction(openAction);

        // If is not a directory, add the Open With... action
        if (!QFileInfo(filePath).isDir()) {
            QAction *openWithAction = new QAction("Open With...", &menu);
            openWithAction->setShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_O));
            connect(openWithAction, &QAction::triggered, [=]() { mainWindow->openWith(filePath); });
            menu.addAction(openWithAction);
        }

        menu.addSeparator();

        QAction *showContentsAction = new QAction(tr("Show Contents"), this);
        showContentsAction->setEnabled(false);
        menu.addAction(showContentsAction);
        connect(showContentsAction, &QAction::triggered, [=]() {
            // Get all selected items
            QModelIndexList selectedIndexes = mainWindow->getCurrentView()->selectionModel()->selectedIndexes();
            for (QModelIndex index : selectedIndexes) {
                mainWindow->openFolderInNewWindow(filePath);
            }
        });
        ApplicationBundle* bundle = new ApplicationBundle(filePath);
        if (bundle->isValid() && bundle->type() != ApplicationBundle::Type::DesktopFile) {
            showContentsAction->setEnabled(true);
        } else {
            showContentsAction->setEnabled(false);
        }
        delete bundle;

        menu.addSeparator();

        QAction *getInfoAction = new QAction(tr("Get Info"), this);
        getInfoAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));
        menu.addAction(getInfoAction);
        connect(getInfoAction, &QAction::triggered, [=]() {
            // Get all selected items
            QModelIndexList selectedIndexes = mainWindow->getCurrentView()->selectionModel()->selectedIndexes();
            for (QModelIndex index : selectedIndexes) {
                // Get the absolute path of the item represented by the index, using the model
                QString filePath = model->data(index, QFileSystemModel::FilePathRole).toString();
                // Destroy the dialog when it is closed
                InfoDialog *infoDialog = InfoDialog::getInstance(filePath, mainWindow);
                infoDialog->setAttribute(Qt::WA_DeleteOnClose);
                infoDialog->show();
            }
        });

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
        menu.addAction(moveToTrashAction);
        moveToTrashAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Backspace));
        connect(moveToTrashAction, &QAction::triggered, [=]() {
            TrashHandler trashHandler;
            trashHandler.moveToTrash({filePath});
        });
        TrashHandler trashHandler;
        if (filePath.startsWith(trashHandler.getTrashPath())) {
            moveToTrashAction->setEnabled(false);
        }

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

void CustomItemDelegate::installEventFilterOnView(QAbstractItemView* view) {
    if (view) {
        view->viewport()->installEventFilter(this);
    }
}

bool CustomItemDelegate::eventFilter(QObject* object, QEvent* event) {
    if (event->type() == QEvent::DragEnter) {
        onDragEnterEvent(static_cast<QDragEnterEvent*>(event));
    } else if (event->type() == QEvent::Drop) {
        onDropEvent(static_cast<QDropEvent*>(event));
    }

    // Continue with default event processing
    return QObject::eventFilter(object, event);
}

void CustomItemDelegate::onDragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void CustomItemDelegate::onDropEvent(QDropEvent* event) {
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        if (!urls.isEmpty()) {
            // Assuming you handle only one file drop, take the first URL
            QString filePath = urls.first().toLocalFile();

            qDebug() << "Dropped file:" << filePath << "at position:" << event->pos();
            // Store the position in extended attribute
            QString coordinates = QString::number(event->pos().x()) + "," + QString::number(event->pos().y());
            qDebug() << "Coordinates:" << coordinates;
            // Set the extended attribute using ExtendedAttributes class
            ExtendedAttributes ea(filePath);
            ea.write("coordinates", coordinates.toUtf8());
            emit fileDropped(filePath, event->pos());
        }
    }
}

void CustomItemDelegate::animationValueChanged(double value)
{
    currentAnimationValue = value;
    qDebug() << "Animation value changed:" << value;

/*
    // Trigger a redraw of the view
    if (QAbstractItemView* view = qobject_cast<QAbstractItemView*>(parent()))
    {
        // Print the class name of the stacked widget that the view is in
        // qDebug() << "Parent widget class name:" << view->parentWidget()->metaObject()->className();
        QWidget* stackedWidget = view->parentWidget();
        // Update the widget in the stacked widget
        // stackedWidget->update(); // FIXME: This destroys the animation, so we should only be doing this when the animation is finished
    } else {
        qDebug() << "Parent widget is not a view";
    }
*/

}

void CustomItemDelegate::animationFinished()
{
    // Animation is finished, perform any cleanup if necessary

    // Ensure the item is drawn normally after the animation is finished
    // You can call the view's update() method here to refresh the view
    // and ensure that all items are drawn normally.
    // For example: view->update();

    qDebug() << "Animation finished";

    // Get the view from the parent widget
    if (QAbstractItemView* view = qobject_cast<QAbstractItemView*>(parent()))
    {
        // Redraw the view
        qDebug() << "Updating view";
        view->viewport()->update();
    } else {
        qDebug() << "Parent widget is not a view";
    }

}

void CustomItemDelegate::stopAnimation()
{
    // Stop the animation timeline
    qDebug() << "Stopping animation";
    animationTimeline->stop();
}

void CustomItemDelegate::startAnimation(const QModelIndex& index)
{
    // If index invalid, stop the animation
    if (!index.isValid())
    {
        qDebug() << "Invalid index. Cannot start animation.";
        return;
    }

    // Set the currently animated index
    m_animatedIndex = index;

    // Start the animation timeline
    animationTimeline->start();
    qDebug() << "Started animation for index:" << index;
}

// Setter function to set the selection model
void CustomItemDelegate::setSelectionModel(QItemSelectionModel* selectionModel)
{
    m_selectionModel = selectionModel;
}

bool CustomItemDelegate::isAnimationRunning() const
{
    return animationTimeline->state() == QTimeLine::Running;
}