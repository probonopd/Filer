#include <QListView>
#include <QPainter>
#include <QPaintEvent>
#include <QDebug>

class CustomListView : public QListView {
public:
    
    CustomListView(QWidget* parent = nullptr);
    ~CustomListView();

    // We are subclassing QListView to access the protected function setPositionForIndex
    // which is used to set the icon coordinates for the icon view.
    // Also we can paint the desktop picture behind the icons.

    // Access the protected function setPositionForIndex directly
    inline void setPositionForIndex(const QPoint& position, const QModelIndex& index) {
        QListView::setPositionForIndex(position, index);
    }

    // Public function to get the item delegate for a given index
    QAbstractItemDelegate* getItemDelegateForIndex(const QModelIndex& index) const {
        return itemDelegate(index);
    }

    void requestDesktopPictureToBePainted(bool request);

    void paintEvent(QPaintEvent* event) override;


private:
    bool should_paint_desktop_picture = false;
};