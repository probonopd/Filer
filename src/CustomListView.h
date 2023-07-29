#include <QListView>

class CustomListView : public QListView {
public:
    CustomListView(QWidget* parent = nullptr) : QListView(parent) {}

    // We are subclassing QListView to access the protected function setPositionForIndex
    // which is used to set the icon coordinates for the icon view.
    // So far we are not making use of this yet, but most likely we will need to use it (like PCManFM-Qt does)
    // If we don't need access to this function, we can remove this class and use QListView directly.

    // Access the protected function setPositionForIndex directly
    inline void setPositionForIndex(const QPoint& position, const QModelIndex& index) {
        QListView::setPositionForIndex(position, index);
    }

    // Public function to get the item delegate for a given index
    QAbstractItemDelegate* getItemDelegateForIndex(const QModelIndex& index) const {
        return itemDelegate(index);
    }
};
