#include "SettingsDialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
        QDialog(parent),
        gridSizeComboBox(new QComboBox)
{
    setWindowTitle("Settings");

    gridSizeComboBox->addItem("Normal");
    gridSizeComboBox->addItem("Wide");

    // Load the initial value from settings and set it in the combo box
    QSettings settings;
    QString initialGridSize = settings.value("gridSize", "Normal").toString();
    gridSizeComboBox->setCurrentText(initialGridSize);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(gridSizeComboBox);
    setLayout(layout);
}

SettingsDialog::~SettingsDialog()
{
}
