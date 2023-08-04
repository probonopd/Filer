#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "CopyManager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    void startCopyWithProgress(const QStringList& fromPaths, const QString& toPath);

private:
    CopyManager copyManager;
    void onCopyFinished();
    void onCopyCanceled();
    void onErrorOccurred(const QString& errorMessage);

};
#endif // MAINWINDOW_H
