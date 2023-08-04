#include "CopyManager.h"
#include "CopyProgressDialog.h"
#include "CopyThread.h"
#include <QDebug>

CopyManager::CopyManager(QObject* parent) : QObject(parent), progressDialog(nullptr), copyThread(nullptr) {

}

CopyManager::~CopyManager() {

}

void CopyManager::copyWithProgress(const QStringList& fromPaths, const QString& toPath) {
    if (copyThread && copyThread->isRunning()) {
        qDebug() << "Another copy operation is already in progress.";
        return;
    }

    progressDialog = new CopyProgressDialog; // Note: No need to specify parent, as it's handled internally by Qt.
    progressDialog->setCopyPaths(fromPaths, toPath);

    copyThread = new CopyThread(fromPaths, toPath); // Note: No need to specify parent, as it's handled internally by Qt.

    // Inform the progress dialog when the operation is finished or canceled
    connect(copyThread, &CopyThread::copyFinished, progressDialog, &CopyProgressDialog::onCopyFinished);
    connect(copyThread, &CopyThread::error, progressDialog, &CopyProgressDialog::onErrorOccurred);
    connect(copyThread, &CopyThread::progress, progressDialog, &CopyProgressDialog::onCopyProgress);

    // Inform the copy thread when the user wants to cancel the operation
    connect(progressDialog, &CopyProgressDialog::cancelCopyRequested, copyThread, &CopyThread::cancelCopyRequested);

    // Connect to the signals of the copy thread to know when the operation is finished or canceled
    connect(copyThread, &CopyThread::progress, this, &CopyManager::onCopyProgress);
    connect(copyThread, &CopyThread::copyFinished, this, &CopyManager::onCopyFinished);
    connect(copyThread, &CopyThread::cancelCopyRequested, this, &CopyManager::onCancelCopy);
    connect(copyThread, &CopyThread::error, this, &CopyManager::onErrorOccurred);

    copyThread->start();

    progressDialog->show();
}

void CopyManager::onCopyProgress(int progress) {
    // You can perform actions related to progress, if needed.
    qDebug() << "Copy progress: " << progress << "%";
}

void CopyManager::onCopyFinished() {
    qDebug() << "CopyManager: Copy operation completed.";
    emit copyFinished();
    if (progressDialog) {
        progressDialog->hide();
        delete progressDialog;
        progressDialog = nullptr;
    }
    if (copyThread) {
        copyThread = nullptr;
    }
}

void CopyManager::onCancelCopy() {
    qDebug() << "CopyManager: Copy operation canceled.";
    emit copyCanceled();
    if (copyThread && copyThread->isRunning()) {
        copyThread->quit();
        copyThread->wait();
        copyThread = nullptr;
    }
    if (progressDialog) {
        progressDialog->hide();
        delete progressDialog;
        progressDialog = nullptr;
    }
}

void CopyManager::onErrorOccurred(const QString& errorMessage) {
    qDebug() << "CopyManager:" << errorMessage;
    emit errorOccured(errorMessage);
    if (copyThread && copyThread->isRunning()) {
        copyThread->quit();
        copyThread->wait();
        copyThread = nullptr;
    }
    if (progressDialog) {
        progressDialog->hide();
        delete progressDialog;
        progressDialog = nullptr;
    }
}