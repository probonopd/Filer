#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QSettings>

/**
 * @file PreferencesDialog.h
 * @brief The Preferences dialog.
 *
 * This class provides a preferences dialog with options to select grid size
 * and set a desktop picture.
 */
class PreferencesDialog : public QDialog
{
Q_OBJECT

public:
    /**
     * @brief getInstance Returns the singleton instance of PreferencesDialog.
     * @param parent The parent widget.
     * @return The singleton instance of PreferencesDialog.
     */
    static PreferencesDialog *getInstance(QWidget *parent = nullptr);

    /**
     * @brief ~PreferencesDialog Destructor.
     */
    ~PreferencesDialog();

signals:
    /**
     * @brief prefsChanged Signal emitted when preferences are changed.
     */
    void prefsChanged();

private slots:
    /**
     * @brief updateSetting Slot to update the selected grid size setting.
     * @param index The index of the selected grid size.
     */
    void updateSetting(int index);

private:
    /**
     * @brief PreferencesDialog Constructor.
     * @param parent The parent widget.
     */
    PreferencesDialog(QWidget *parent = nullptr);

    QComboBox *gridSizeComboBox;
    QPushButton *selectPictureButton;
    QGridLayout *layout;

    /**
     * @brief loadSetting Loads the preferences settings and updates the UI.
     */
    void loadSetting();

    /**
     * @brief onSelectPictureButtonClicked Slot for handling picture selection button click.
     */
    void onSelectPictureButtonClicked();
};

#endif // PREFERENCESDIALOG_H
