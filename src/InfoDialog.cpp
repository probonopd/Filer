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

#include "InfoDialog.h"
#include "ui_InfoDialog.h"

#include <QDateTime>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QAction>
#include "CustomFileIconProvider.h"
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QDebug>

InfoDialog::InfoDialog(const QString &filePath, QWidget *parent) :
        QDialog(parent),
        ui(new Ui::InfoDialog),
        filePath(filePath),
        fileInfo(filePath)
{
    ui->setupUi(this);

    setWindowTitle(filePath.mid(filePath.lastIndexOf("/") + 1) + " Info");

    setupInformation();

    // Destroy the dialog when it is closed
    setAttribute(Qt::WA_DeleteOnClose);

    QAction *closeAction = new QAction(this);
    closeAction->setShortcut(Qt::CTRL + Qt::Key_W);
    connect(closeAction, &QAction::triggered, this, &InfoDialog::close);
    addAction(closeAction);


}

InfoDialog::~InfoDialog()
{
    delete ui;
}

void InfoDialog::setupInformation()
{
    qDebug() << fileInfo.absoluteFilePath();
    QIcon icon = QIcon::fromTheme("unknown");

    // Make the window unresizable horizontally
    setMinimumWidth(400);
    setMaximumWidth(400);

    ui->iconInfo->setPixmap(icon.pixmap(128, 128));
    ui->iconInfo->setStyleSheet("border: 1px solid grey;");
    ui->iconInfo->setFixedSize(128, 128);
    ui->iconInfo->setAlignment(Qt::AlignCenter);
    // Make the icon selectable
    ui->iconInfo->setFocusPolicy(Qt::ClickFocus);

    // Make it so that one can copy and paste the icon
    ui->iconInfo->setContextMenuPolicy(Qt::ActionsContextMenu);
    QAction *copyAction = new QAction(tr("Copy"), this);
    copyAction->setShortcut(Qt::CTRL + Qt::Key_C);
    // connect(copyAction, &QAction::triggered, this, &InfoDialog::copyIcon);
    ui->iconInfo->addAction(copyAction);
    QAction *pasteAction = new QAction(tr("Paste"), this);
    pasteAction->setShortcut(Qt::CTRL + Qt::Key_V);
    // connect(pasteAction, &QAction::triggered, this, &InfoDialog::pasteIcon);
    ui->iconInfo->addAction(pasteAction);

    ui->pathInfo->setText(filePath);
    ui->pathInfo->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

    ui->sizeInfo->setText(QString::number(fileInfo.size()) + " bytes");

    ui->createdInfo->setText(fileInfo.created().toString(Qt::DefaultLocaleLongDate));

    ui->modifiedInfo->setText(fileInfo.lastModified().toString(Qt::DefaultLocaleLongDate));

    QFile::Permissions permissions = fileInfo.permissions();

    QString permissionsString = getPermissionsString(permissions);

    ui->permissionsInfo->setText(permissionsString);

    if (fileInfo.isDir()) {
        // In the GUI, we always use the word "Folder" instead of "Directory"
        ui->typeInfo->setText(tr("Folder"));
    } else if (fileInfo.isFile()) {
        // In the GUI, we always use the word "Document" instead of "File"
        ui->typeInfo->setText(tr("Document"));
    } else {
        ui->typeInfo->setText(tr("Unknown"));
    }
}

void InfoDialog::openFile()
{
    QMessageBox::warning(this, tr("Error"), tr("Not implemented yet!"));
}

QString InfoDialog::getPermissionsString(QFile::Permissions permissions)
{
    QString result;
    if (permissions & QFile::ReadOwner) result += "r";
    else result += "-";
    if (permissions & QFile::WriteOwner) result += "w";
    else result += "-";
    if (permissions & QFile::ExeOwner) result += "x";
    else result += "-";
    if (permissions & QFile::ReadGroup) result += "r";
    else result += "-";
    if (permissions & QFile::WriteGroup) result += "w";
    else result += "-";
    if (permissions & QFile::ExeGroup) result += "x";
    else result += "-";
    if (permissions & QFile::ReadOther) result += "r";
    else result += "-";
    if (permissions & QFile::WriteOther) result += "w";
    else result += "-";
    if (permissions & QFile::ExeOther) result += "x";
    else result += "-";

    return result;
}