#ifndef COPYPROGRESSDIALOG_H
#define COPYPROGRESSDIALOG_H

#include <QDialog>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QKeyEvent>

class CopyProgressDialog : public QDialog {
    Q_OBJECT

public:
    explicit CopyProgressDialog(QWidget* parent = nullptr);
    ~CopyProgressDialog();
    void setCopyPaths(const QStringList& fromPaths, const QString& toPath);
    void updateProgress(int value);


public slots:
    void onCopyProgress(int progress);
    void onCopyFinished();
    void onCancelCopy();
    void onErrorOccurred(const QString& errorMessage);

    signals:
            void cancelCopyRequested();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    QLabel* fromLabel;
    QLabel* toLabel;
    QProgressBar* progressBar;
    QPushButton* cancelButton;
    QStringList fromFilePaths;
    QString toFilePath;

};

#endif // COPYPROGRESSDIALOG_H
