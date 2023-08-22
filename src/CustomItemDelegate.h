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

/**
 * @file CustomItemDelegate.h
 * @brief The CustomItemDelegate class.
 */

#ifndef CUSTOMITEMDELEGATE_H
#define CUSTOMITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QFileSystemModel>
#include <QObject>
#include <QAction>
#include <QMenu>
#include <QMimeData>
#include "CustomFileSystemModel.h"
#include <QTimeLine>
#include <QItemSelectionModel>
#include <QStandardItemModel>
#include "CustomFileIconProvider.h"

/// Add a custom role to store the delegate position
enum CustomItemDelegateRole {
    DelegatePositionRole = Qt::UserRole + 1
};

/**
 * @class CustomItemDelegate
 * @brief A custom delegate for rendering items in views.
 *
 * The CustomItemDelegate class provides a custom delegate for rendering items in views,
 * such as QTreeView or QListView. It extends QStyledItemDelegate and adds custom behavior
 * like animations, context menus, and drag-and-drop handling.
 */
class CustomItemDelegate : public QStyledItemDelegate
{
Q_OBJECT

public:
    /**
     * @brief Constructs a CustomItemDelegate object.
     * @param parent The parent QObject.
     * @param fileSystemModel A pointer to the CustomFileSystemModel.
     */
    explicit CustomItemDelegate(QObject* parent = nullptr, QAbstractProxyModel* fileSystemModel = nullptr);

    // Destructor
    ~CustomItemDelegate();

    /**
     * @brief Returns the displayed text for the item at the specified index.
     * @param value The data value.
     * @param locale The locale to be used.
     * @return The formatted display text.
     */
    QString displayText(const QVariant &value, const QLocale &locale) const override;

    /**
     * @brief Renders the item's appearance using a QPainter.
     * @param painter The QPainter object to paint with.
     * @param option The style options for rendering.
     * @param index The model index to be rendered.
     */
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    /**
     * @brief Installs event filters on the specified view to intercept events.
     * @param view The view on which to install the event filters.
     */
    // void installEventFilterOnView(QAbstractItemView* view);

    /**
     * @brief Overrides the event filter to handle events for the delegate.
     * @param object The QObject that sent the event.
     * @param event The event to be filtered.
     * @return True if the event was handled, otherwise false.
     */
    // bool eventFilter(QObject* object, QEvent* event) override;

    /**
     * @brief Sets the selection model for the delegate.
     * @param selectionModel The QItemSelectionModel to use for selection.
     */
    void setSelectionModel(QItemSelectionModel* selectionModel);

    /**
     * @brief Overrides the sizeHint method to set the size of the delegate items.
     * @param option The style options for the item.
     * @param index The index of the item.
     * @return The preferred size of the item.
     */
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        // Get the current size
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        return QSize(option.rect.width(), size.height());
    }

public slots:
    /**
     * @brief Animates the value change for the animation.
     * @param value The new animation value.
     */
    void animationValueChanged(double value);

    /**
     * @brief Slot called when the animation has finished.
     */
    void animationFinished();

    /**
     * @brief Stops the currently running animation.
     */
    void stopAnimation();

    /**
     * @brief Starts an animation for the specified index.
     * @param index The index of the item to animate.
     */
    void startAnimation(const QModelIndex& index);

    /**
     * @brief Checks if an animation is currently running.
     * @return True if an animation is running, otherwise false.
     */
    bool isAnimationRunning() const;

signals:
    /**
     * @brief Signal emitted when a file is dropped onto the delegate item.
     * @param filePath The path of the dropped file.
     * @param iconPosition The position of the icon.
     */
    void fileDropped(const QString& filePath, const QPoint& iconPosition);

protected:
    // Override the editorEvent() function to handle right-click events for the context menu
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

private:
    // Private member variable to hold a pointer to the QFileSystemModel object
    QAbstractProxyModel *m_fileSystemModel;

    // We use this to flash the icon if the item was double-clicked
    bool iconShown = false;
    bool iconVisible = false;
    int flashCount = 0;

    // Context menu
    // It is generally not a good idea to create a context menu in an ItemDelegate object,
    // because the ItemDelegate object is responsible for drawing the items in the view,
    // and it is not responsible for handling user interactions with the items.
    // We should probably fix this in a future version of the application.
    QMenu menu;

    QTimeLine* animationTimeline;

    CustomFileIconProvider* iconProvider;

    qreal currentAnimationValue;

    // Member variables to store the current index and option
    mutable QModelIndex m_currentIndex;
    QModelIndex m_animatedIndex; // The index of the item that should currently be animated
    mutable QStyleOptionViewItem m_currentOption;

    QItemSelectionModel* m_selectionModel;

private slots:
    // Slot to handle drag enter events
    // void onDragEnterEvent(QDragEnterEvent* event);

    // Slot to handle drop events
    // void onDropEvent(QDropEvent* event);
};

#endif // CUSTOMITEMDELEGATE_H
