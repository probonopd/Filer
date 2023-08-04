/*-
 * Copyright (c) 2022-23 Simon Peter <probono@puredarwin.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "InformationDialog.h"
#include "ui_InformationDialog.h"

#include <QDateTime>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>

InformationDialog::InformationDialog(const QString &filePath, QWidget *parent) :
        QDialog(parent),
        ui(new Ui::InformationDialog),
        filePath(filePath),
        fileInfo(filePath)
{
    ui->setupUi(this);
    setWindowTitle(tr("Information"));

    setupInformation();

    // Destroy the dialog when it is closed
    setAttribute(Qt::WA_DeleteOnClose);

    connect(ui->openButton, &QPushButton::clicked, this, &InformationDialog::openFile);

    QAction *closeAction = new QAction(this);
    closeAction->setShortcut(Qt::CTRL + Qt::Key_W);
    connect(closeAction, &QAction::triggered, this, &InformationDialog::close);
    addAction(closeAction);


}

InformationDialog::~InformationDialog()
{
    delete ui;
}

void InformationDialog::setupInformation()
{
    ui->pathLabel->setText(filePath);
    ui->sizeLabel->setText(QString::number(fileInfo.size()) + " bytes");
    ui->createdLabel->setText(fileInfo.created().toString(Qt::DefaultLocaleLongDate));
    ui->modifiedLabel->setText(fileInfo.lastModified().toString(Qt::DefaultLocaleLongDate));
    QFile::Permissions permissions = fileInfo.permissions();
    QString permissionsString = getPermissionsString(permissions);
    ui->permissionsLabel->setText(permissionsString);

    if (fileInfo.isDir()) {
        // In the GUI, we always use the word "Folder" instead of "Directory"
        ui->typeLabel->setText(tr("Folder"));
    } else if (fileInfo.isFile()) {
        // In the GUI, we always use the word "Document" instead of "File"
        ui->typeLabel->setText(tr("Document"));
    } else {
        ui->typeLabel->setText(tr("Unknown"));
    }

    if (fileInfo.isFile()) {
        ui->openButton->setEnabled(true);
        connect(ui->openButton, &QPushButton::clicked, this, &InformationDialog::openFile);
    } else {
        ui->openButton->setEnabled(false);
    }
}

void InformationDialog::openFile()
{
    QMessageBox::warning(this, tr("Error"), tr("Not implemented yet!"));
}

QString InformationDialog::getPermissionsString(QFile::Permissions permissions)
{
    QString result;
    if (permissions & QFile::ReadOwner) result += "r";
    else result += "-";
    if (permissions & QFile::WriteOwner) result += "w";
    else result += "-";
    if (permissions & QFile::ExeOwner) result += "x";
    else result += "-";
    // Repeat the above for group and others permissions

    return result;
}