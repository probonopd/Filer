#include "PreferencesDialog.h"
#include <QSettings>
#include <QDebug>
#include <QPushButton>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QSpacerItem>
#include <QFile>
#include <QMessageBox>
#include <QDropEvent>
#include <QMimeData>
#include <QApplication>
#include <QScreen>
#include <QDesktopWidget>
#include <QDir>

// Grid sizes
const QMap<QString, int> GridSizeMapping = {
        {QObject::tr("Normal"), 120},
        {QObject::tr("Wide"), 240}
};

PreferencesDialog *PreferencesDialog::getInstance(QWidget *parent)
{
    static PreferencesDialog instance(parent);
    // Raise, unminimize, and activate the dialog
    instance.raise();
    instance.showNormal();
    instance.activateWindow();
    return &instance;
}

PreferencesDialog::PreferencesDialog(QWidget *parent) :
        QDialog(parent),
        gridSizeComboBox(new QComboBox),
        imageLabel(new QLabel)
{

    setWindowTitle(tr("Preferences"));

    QLabel *gridSizeLabel = new QLabel(tr("Grid size:"));

    gridSizeComboBox->addItems(GridSizeMapping.keys());

    QGridLayout *layout = new QGridLayout;

    // Desktop picture
    layout->addWidget(new QLabel(tr("Desktop Picture:")), 0, 0);

    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    double aspectRatio = (double) screenGeometry.width() / screenGeometry.height();
    imageLabel->setFixedSize(200, 200 / aspectRatio);
    imageLabel->installEventFilter(this);
    imageLabel->setAcceptDrops(true);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setStyleSheet("border: 2px dashed gray;");
    layout->addWidget(imageLabel, 0, 1);

    // Grid size
    layout->addWidget(gridSizeLabel, 1, 0);
    layout->addWidget(gridSizeComboBox, 1, 1);
    layout->setColumnStretch(1, 1);
    connect(gridSizeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PreferencesDialog::updateSetting);

    // setRowStretch to 0 for all but the last row
    for (int i = 0; i < layout->rowCount() - 1; ++i) {
        layout->setRowStretch(i, 0);
    }
    // For the last row, set the stretch factor to 1 and add a spacer item
    layout->setRowStretch(layout->rowCount() - 1, 1);
    layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding),
                    layout->rowCount(), 0, 1, layout->columnCount());

    setLayout(layout);

    resize(400, 180);

    // Load the initial setting value
    loadSetting();
}

PreferencesDialog::~PreferencesDialog()
{
}

void PreferencesDialog::loadSetting()
{
    qDebug() << "PreferencesDialog::loadSetting()";
    QSettings settings;

    // Desktop picture
    QString desktopPicturePath = settings.value("desktopPicture").toString();
    QPixmap *pixmap = new QPixmap(desktopPicturePath);
    if (!pixmap->isNull()) {
        imageLabel->setPixmap(pixmap->scaled(imageLabel->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    } else {
        // The file doesn't exist, so clear the setting
        settings.remove("desktopPicture");
    }
    if (!desktopPicturePath.isEmpty()) {
        // Check if the file exists
        if (QFile::exists(desktopPicturePath)) {
        } else {
            // The file doesn't exist, so clear the setting
            settings.remove("desktopPicture");
        }
    }

    // Grid size
    // Default is the first one from GridSizeMapping
    int defaultIndex = 0;
    // Read the setting, and if it's found, set the combo box's current index based on the stored gridSize
    int gridSize = settings.value("gridSize", GridSizeMapping[GridSizeMapping.keys().at(0)]).toInt();
    int index = gridSizeComboBox->findText(GridSizeMapping.key(gridSize));
    if (index != -1) {
        gridSizeComboBox->setCurrentIndex(index);
    } else {
        gridSizeComboBox->setCurrentIndex(defaultIndex);
    }

    qDebug() << "Preferences have been read from" << settings.fileName();
}

void PreferencesDialog::updateSetting(int index)
{
    qDebug() << "PreferencesDialog::updateSetting()";
    QSettings settings;

    // Set the combo box's current index based on the stored gridSize
    int gridSize = GridSizeMapping[gridSizeComboBox->currentText()];
    settings.setValue("gridSize", gridSize);

    qDebug() << "Preferences have been written to" << settings.fileName();

    // Emit a signal to the application that QSettings have been changed
    emit prefsChanged();
}

void PreferencesDialog::loadDesktopPicture()
{
    // Default to the path of the currently loaded desktop picture
    QSettings settings;
    QString defaultPath = settings.value("desktopPicture").toString();
    // If the path is empty, default to the user's home directory
    if (defaultPath.isEmpty()) {
        defaultPath = QDir::homePath();
    }
    // Open a file dialog to select an image
    QString filePath = QFileDialog::getOpenFileName(this, tr("Select an image"), defaultPath, tr("Images (*.png *.jpg)"));
    if (!filePath.isEmpty()) {
        QPixmap pixmap(filePath);
        if (!pixmap.isNull()) {
            imageLabel->setPixmap(pixmap.scaled(imageLabel->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
            // Save the setting
            QSettings settings;
            settings.setValue("desktopPicture", filePath);
            // Emit a signal to the application that QSettings have been changed
            emit prefsChanged();
        }
    }
}

bool PreferencesDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == imageLabel) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                // Handle the left mouse button click here
                // You can call your slot function or emit a signal as needed
                loadDesktopPicture(); // Example: Call the loadDesktopPicture function
                return true;
            }
        } else if (event->type() == QEvent::DragEnter) {
            QDragEnterEvent *dragEnterEvent = static_cast<QDragEnterEvent*>(event);
            const QMimeData *mimeData = dragEnterEvent->mimeData();
            if (mimeData->hasUrls() && mimeData->urls().count() == 1) {
                QString filePath = mimeData->urls().first().toLocalFile();
                if (filePath.endsWith(".png", Qt::CaseInsensitive) || filePath.endsWith(".jpg", Qt::CaseInsensitive)) {
                    dragEnterEvent->acceptProposedAction();
                    return true;
                }
            }
        } else if (event->type() == QEvent::Drop) {
            QDropEvent *dropEvent = static_cast<QDropEvent*>(event);
            const QMimeData *mimeData = dropEvent->mimeData();
            if (mimeData->hasUrls() && mimeData->urls().count() == 1) {
                QString filePath = mimeData->urls().first().toLocalFile();
                if (filePath.endsWith(".png", Qt::CaseInsensitive) || filePath.endsWith(".jpg", Qt::CaseInsensitive)) {
                    QPixmap pixmap(filePath);
                    if (!pixmap.isNull()) {
                        imageLabel->setPixmap(pixmap.scaled(imageLabel->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
                        // Save the setting
                        QSettings settings;
                        settings.setValue("desktopPicture", filePath);
                        // Emit a signal to the application that QSettings have been changed
                        emit prefsChanged();
                    }
                    dropEvent->acceptProposedAction();
                    return true;
                }
            }
        }
    }

    return false; // Let the base class handle the event
}