#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QSettings>

class PreferencesDialog : public QDialog
{
Q_OBJECT

public:
    static PreferencesDialog *getInstance(QWidget *parent = nullptr);
    ~PreferencesDialog();

signals:
    void prefsChanged();

private slots:
    void updateSetting(int index);

private:
    PreferencesDialog(QWidget *parent = nullptr);

    QComboBox *gridSizeComboBox;
    QPushButton *selectPictureButton;
    QGridLayout *layout;

    void loadSetting();

    void onSelectPictureButtonClicked();
};

#endif // PREFERENCESDIALOG_H
