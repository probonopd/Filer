#include "MainWindow.h"
#include <QDebug>
#include <QApplication>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {

    setWindowTitle("MainWindow");
    setFixedSize(400, 150);
    // Apparently every Qt application needs a MainWindow
    // but we don't need to show it
    // show();
}

void MainWindow::startCopyWithProgress(const QStringList& fromPaths, const QString& toPath) {
    copyManager.copyWithProgress(fromPaths, toPath);

    // Register a callback to know when the copy is finished
    connect(&copyManager, &CopyManager::copyFinished, this, &MainWindow::onCopyFinished);
}

void MainWindow::onCopyFinished() {
    qDebug() << "Copy finished!";
    // QCoreApplication::quit();
}

void MainWindow::onCopyCancelled() {
    qDebug() << "Copy cancelled!";
    // QCoreApplication::quit();
}