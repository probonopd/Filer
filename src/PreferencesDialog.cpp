#include "PreferencesDialog.h"
#include <QSettings>
#include <QDebug>
#include <QPushButton>
#include <QFileDialog>
#include <QFileInfo>
#include <QLabel>
#include <QSpacerItem>
#include <QFile>

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
        selectPictureButton(new QPushButton(tr("Select Picture")))
{

    setWindowTitle(tr("Preferences"));

    QLabel *gridSizeLabel = new QLabel(tr("Grid size:"));

    gridSizeComboBox->addItems(GridSizeMapping.keys());

    QGridLayout *layout = new QGridLayout;

    // Desktop picture
    layout->addWidget(new QLabel(tr("Desktop Picture:")), 0, 0);
    layout->addWidget(selectPictureButton, 0, 1);
    onSelectPictureButtonClicked();

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

void PreferencesDialog::onSelectPictureButtonClicked() {
    connect(selectPictureButton, &QPushButton::clicked, this, [this]() {
        // Open a file dialog
        QString desktopPicturePath = QFileDialog::getOpenFileName(this, tr("Select Picture"), QString(),
                                                                  tr("Images (*.png *.jpg *.jpeg *.bmp)"));
        if (!desktopPicturePath.isEmpty()) {
            // Set the button's text to the file name
            selectPictureButton->setText(QFileInfo(desktopPicturePath).fileName());

            // Save the setting
            QSettings settings;
            settings.setValue("desktopPicture", desktopPicturePath);

            // Emit a signal to the application that QSettings have been changed
            emit prefsChanged();
        }
    });
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
    if (!desktopPicturePath.isEmpty()) {
        // Check if the file exists
        if (QFile::exists(desktopPicturePath)) {
            // Set the button's text to the file name
            selectPictureButton->setText(QFileInfo(desktopPicturePath).fileName());
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