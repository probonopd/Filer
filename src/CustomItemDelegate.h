#ifndef CUSTOMITEMDELEGATE_H
#define CUSTOMITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QFileSystemModel>
#include <QObject>
#include <QAction>
#include <QMenu>
#include <QMimeData>

// Add a custom role to store the delegate position
enum CustomItemDelegateRole {
    DelegatePositionRole = Qt::UserRole + 1
};

// Define the CustomItemDelegate class, which is derived from the QStyledItemDelegate class
class CustomItemDelegate : public QStyledItemDelegate
{
Q_OBJECT

public:
    CustomItemDelegate(QObject *parent, QFileSystemModel *fileSystemModel);

    // Destructor
    ~CustomItemDelegate();

    QString displayText(const QVariant &value, const QLocale &locale) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    // Install event filters on your view using this function
    void installEventFilterOnView(QAbstractItemView* view);

    bool eventFilter(QObject* object, QEvent* event) override;

signals:
    // Define a signal to emit the file path and icon coordinates when a file is dropped
    void fileDropped(const QString& filePath, const QPoint& iconPosition);

protected:
    // Override the editorEvent() function to handle right-click events for the context menu
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;

private:
    // Private member variable to hold a pointer to the QFileSystemModel object
    QFileSystemModel *m_fileSystemModel;

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

private slots:
    // Slot to handle drag enter events
    void onDragEnterEvent(QDragEnterEvent* event);

    // Slot to handle drop events
    void onDropEvent(QDropEvent* event);
};

#endif // CUSTOMITEMDELEGATE_H
