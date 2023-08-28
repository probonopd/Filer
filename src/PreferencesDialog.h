#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QVBoxLayout>
#include <QSettings>

class SettingsDialog : public QDialog
{
Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

private:
    QComboBox *gridSizeComboBox;
    QSettings settings; // QSettings instance to manage settings
};

#endif // SETTINGSDIALOG_H
