#include "MainWindow.h"
#include <QDebug>
#include <QApplication>
#include <QMessageBox>

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
    // Register a callback to know when the copy was cancelled or an error occurred
    connect(&copyManager, &CopyManager::copyCanceled, this, &MainWindow::onCopyCanceled);
    // Register a callback to know when an error occurred
    connect(&copyManager, &CopyManager::errorOccured, this, &MainWindow::onErrorOccurred);
}

void MainWindow::onCopyFinished() {
    qDebug() << "MainWindow: Copy finished!";
    QCoreApplication::quit();
}

void MainWindow::onCopyCanceled() {
    qDebug() << "MainWindow: Copy canceled!";
    // Exit with an error code
    QCoreApplication::exit(1);
}

void MainWindow::onErrorOccurred(const QString& errorMessage) {
    qDebug() << "MainWindow: Error occurred: " << errorMessage;
    QMessageBox::critical(this, tr("Error"), errorMessage);
    // Exit with an error code
    QCoreApplication::exit(1);
}