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

// Add a custom role to store the delegate position
enum CustomItemDelegateRole {
    DelegatePositionRole = Qt::UserRole + 1
};

// Define the CustomItemDelegate class, which is derived from the QStyledItemDelegate class
class CustomItemDelegate : public QStyledItemDelegate
{
Q_OBJECT

public:
    explicit CustomItemDelegate(QObject* parent = nullptr, CustomFileSystemModel* fileSystemModel = nullptr);

    // Destructor
    ~CustomItemDelegate();

    QString displayText(const QVariant &value, const QLocale &locale) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    // Install event filters on your view using this function
    void installEventFilterOnView(QAbstractItemView* view);

    bool eventFilter(QObject* object, QEvent* event) override;

    // We need to know the selection model to be able to animate the selected item instead of all items
    void setSelectionModel(QItemSelectionModel* selectionModel);

    // Override the sizeHint method to set the size for the delegate to the width available in the view
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        // Get current size
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        return QSize(option.rect.width(), size.height());
    }

public slots:
    void animationValueChanged(double value);
    void animationFinished();
    void stopAnimation();
    void startAnimation(const QModelIndex& index);
    bool isAnimationRunning() const;

signals:
    // Define a signal to emit the file path and icon coordinates when a file is dropped
    void fileDropped(const QString& filePath, const QPoint& iconPosition);

protected:
    // Override the editorEvent() function to handle right-click events for the context menu
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

private:
    // Private member variable to hold a pointer to the QFileSystemModel object
    CustomFileSystemModel *m_fileSystemModel;

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

    qreal currentAnimationValue;

    // Member variables to store the current index and option
    mutable QModelIndex m_currentIndex;
    QModelIndex m_animatedIndex; // The index of the item that should currently be animated
    mutable QStyleOptionViewItem m_currentOption;

    QItemSelectionModel* m_selectionModel;

private slots:
    // Slot to handle drag enter events
    void onDragEnterEvent(QDragEnterEvent* event);

    // Slot to handle drop events
    void onDropEvent(QDropEvent* event);
};

#endif // CUSTOMITEMDELEGATE_H
