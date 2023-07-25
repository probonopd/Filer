#include "CombinedIconCreator.h"
#include <QImage>
#include <QColor>
#include <QIcon>
#include <QPainter>
#include <QHash>

bool isVibrantColor(const QColor& color) {
    // Threshold values to define vibrant colors
    const int minSaturation = 30; // Minimum saturation (0-255) for vibrant colors
    const int minLightness = 70;  // Minimum lightness (0-255) for vibrant colors

    return color.saturation() > minSaturation && color.lightness() > minLightness;
}

QColor CombinedIconCreator::findDominantColor(const QPixmap& pixmap) {
    QImage image = pixmap.toImage();

    QHash<QRgb, int> colorCounts;

    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QRgb pixelColor = image.pixel(x, y);
            QColor color(pixelColor);
            if (isVibrantColor(color)) {
                colorCounts[pixelColor]++;
            }
        }
    }

    int maxCount = 0;
    QRgb dominantColor = 0;

    for (const auto& color : colorCounts.keys()) {
        int count = colorCounts.value(color);
        if (count > maxCount) {
            maxCount = count;
            dominantColor = color;
        }
    }

    // If the dominant color is black or very dark, return neutral gray
    if (QColor(dominantColor).lightness() < 10) {
        return QColor(Qt::lightGray);
    }

    // Same if the dominant color is white or very light
    if (QColor(dominantColor).lightness() > 220) {
        return QColor(Qt::lightGray);
    }

    return QColor(dominantColor);
}

QIcon CombinedIconCreator::createCombinedIcon(const QIcon& applicationIcon) {
    QIcon documentIcon = QIcon::fromTheme("document");
    QPixmap document_pixmap = documentIcon.pixmap(32, 32);
    QPixmap application_pixmap = applicationIcon.pixmap(24, 24);

    QColor dominantColor = findDominantColor(application_pixmap);

    QPixmap combined_icon(32, 32);
    combined_icon.fill(Qt::transparent); // Fill the pixmap with transparent background

    QPainter painter(&combined_icon);

    QPixmap coloredPixmap(document_pixmap.size());
    coloredPixmap.fill(Qt::transparent);
    painter.drawPixmap(0, 0, document_pixmap);

    // Draw a semi-transparent rectangle at the bottom of the icon
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(7, document_pixmap.height() - 7, document_pixmap.width() - 14, 3, dominantColor);

    // Draw the document icon with a color overlay using CompositionMode
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawPixmap(0, 0, coloredPixmap);

    // Draw the application icon
    // Move 5 pixels up to make room for the color rectangle
    painter.drawPixmap(8, 8-5, application_pixmap);

    painter.end();

    return QIcon(combined_icon);
}
