#include "CopyProgressDialog.h"
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDebug>
#include <QMessageBox>

CopyProgressDialog::CopyProgressDialog(QWidget* parent) : QDialog(parent), fromLabel(nullptr), toLabel(nullptr),
                                                          progressBar(nullptr), cancelButton(nullptr) {

    fromLabel = new QLabel(this);
    toLabel = new QLabel(this);
    progressBar = new QProgressBar(this);
    cancelButton = new QPushButton("Cancel", this);

    connect(cancelButton, &QPushButton::clicked, this, &CopyProgressDialog::onCancelCopy);

    // Main layout for the dialog
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(fromLabel);
    mainLayout->addWidget(toLabel);
    mainLayout->addWidget(progressBar);

    // Layout for the cancel button in the bottom right corner
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelButton);

    // Set the main layout for the dialog
    setLayout(mainLayout);

    // Add the button layout to the main layout
    mainLayout->addLayout(buttonLayout);

    setWindowTitle("Copying...");
    setFixedSize(400, 150);
}

CopyProgressDialog::~CopyProgressDialog() {
    qDebug() << "CopyProgressDialog destructor called.";
}

void CopyProgressDialog::setCopyPaths(const QStringList& fromPaths, const QString& toPath) {
    QString fromPathsText = "Copying from: " + fromPaths.join(", ");
    fromLabel->setText(fromPathsText);
    toLabel->setText("Copying to: " + toPath);
}

void CopyProgressDialog::onCopyProgress(int progress) {
    progressBar->setValue(progress);
}

void CopyProgressDialog::onCopyFinished() {
    this->close();
}

// This slot is called when the cancel button is clicked
void CopyProgressDialog::onCancelCopy() {
    qDebug() << "CopyProgressDialog::onCancelCopy";
    // Emit the cancelCopyRequested signal to indicate the copy was canceled
    emit cancelCopyRequested();
    this->close();
}

// This event is called when the user presses a key; we use it to detect the Escape key
void CopyProgressDialog::keyPressEvent(QKeyEvent* event) {
    qDebug() << "CopyProgressDialog::keyPressEvent";
    if (event->key() == Qt::Key_Escape) {
        emit cancelCopyRequested();
        this->close();
        event->accept();
    } else {
        event->ignore();
    }
}

// This event is called when the user clicks the 'X' button in the title bar
// and when the window gets closed because the progress is finished
void CopyProgressDialog::closeEvent(QCloseEvent* event) {
    qDebug() << "CopyProgressDialog::closeEvent";
    // Check if the copy is finished
    if (progressBar->value() == progressBar->maximum()) {
        // In this case, the window got closed because the copy is finished,
        // so we don't emit the cancelCopyRequested signal
        event->accept();
        return;
    }
    // In all other cases, we emit the cancelCopyRequested signal
    emit cancelCopyRequested();
    this->close();
    event->accept();
}

void CopyProgressDialog::onErrorOccurred(const QString& errorMessage) {
    hide();
    QMessageBox::warning(this, tr("Error"), errorMessage);
    close();
}