//
// Created by user on 27.07.23.
//

#include "CustomListView.h"

CustomListView::CustomListView(QWidget* parent) : QListView(parent) {
    should_paint_desktop_picture = false;
}

CustomListView::~CustomListView() {
    qDebug() << "CustomListView::~CustomListView";
}

void CustomListView::requestDesktopPictureToBePainted(bool request) {
    qDebug() << "CustomListView::requestDesktopPictureToBePainted" << request;
    should_paint_desktop_picture = request;
}

void CustomListView::paintEvent(QPaintEvent* event)
{

    if(!should_paint_desktop_picture) {
        QListView::paintEvent(event);
        return;
    }

    // setContentsMargins(0, 10, 0, 0);
    setContentsMargins(0, 10, 0, 0);
    setSpacing(10);

    // Add 1cm of padding to the top, so that the items
    // are 1cm below the top of the window and won't get
    // covered by the Menu

    // Get the viewport of the list view
    viewport()->setContentsMargins(0, 10, 0, 0);
    // Why does this not work?
    // Because the viewport is not a QWidget, but a QAbstractScrollArea
    // So we need to use setContentsMargins on the viewport instead
    setContentsMargins(0, 10, 0, 0);
    
    QPainter painter(viewport());

    // Save the painter state
    painter.save();

    QString desktopPicture = "/usr/local/share/slim/themes/default/background.jpg";

    // If exists, use the user's desktop picture
    if (QFile::exists(desktopPicture)) {
        // Draw the desktop picture
        QPixmap background(desktopPicture);
        background = background.scaled(this->size(), Qt::KeepAspectRatioByExpanding);
        painter.drawPixmap(0, 0, background);

        // Draw a grey background over it to make it more muted; TODO: Remove this and fix the desktop picture instead
        painter.fillRect(this->rect(), QColor(128, 128, 128, 128));
    } else {
        // If not, use a solid color gradient
        QLinearGradient gradient(0, 0, 0, this->height());
        gradient.setColorAt(0, QColor(128-30, 128, 128+30));
        gradient.setColorAt(1, QColor(48-30, 48, 48+30));
        painter.fillRect(this->rect(), gradient);
    }

    // Draw a rectangle with a gradient at the top of the window
    // so that the Menu is more visible
    QPen pen(Qt::NoPen);
    painter.setPen(pen);
    QRect rect(0, 0, this->width(), 44);
    QLinearGradient gradient(0, 22, 0, 44);
    gradient.setColorAt(0, QColor(0, 0, 0, 50));
    gradient.setColorAt(1, QColor(0, 0, 0, 0));
    painter.fillRect(rect, gradient);

    // Restore the painter state
    painter.restore();

    // Call super class paintEvent to draw the items
    QListView::paintEvent(event);
}