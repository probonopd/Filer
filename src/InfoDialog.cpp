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
#include <QMimeDatabase>
#include "ApplicationBundle.h"
#include "CustomFileSystemModel.h"
#include "CustomItemDelegate.h"
#include <QProcess>
#include <QClipboard>
#include <QMouseEvent>
#include <QSortFilterProxyModel>
#include "Mountpoints.h"

QMap<QString, InfoDialog*> InfoDialog::instances; // All instances of InfoDialog share this map

// Use this to get an instance of InfoDialog (existing or new)
InfoDialog* InfoDialog::getInstance(const QString &filePath, QWidget *parent)
{
    if (instances.contains(filePath)) {
        InfoDialog *existingDialog = instances.value(filePath);
        existingDialog->raise();
        existingDialog->activateWindow();
        return existingDialog;
    }

    InfoDialog *newDialog = new InfoDialog(filePath, parent);
    instances.insert(filePath, newDialog);
    return newDialog;
}

InfoDialog::InfoDialog(const QString &filePath, QWidget *parent) :
        QDialog(parent),
        ui(new Ui::InfoDialog),
        filePath(filePath),
        fileInfo(filePath)
{

    this->filePath = filePath;

    // Check if an instance already exists for this file
    if (instances.contains(filePath)) {
        InfoDialog *existingDialog = instances.value(filePath);
        existingDialog->raise(); // Bring the existing dialog to the front
        existingDialog->activateWindow();
        reject(); // Reject the new instance before showing it
        return;
    }

    instances.insert(filePath, this);

    ui->setupUi(this);

    // Connect the fileChanged slot to the fileChanged signal of the watcher
    connect(&fileWatcher, &QFileSystemWatcher::fileChanged, this, &InfoDialog::fileChanged);
    fileWatcher.addPath(filePath);

    setWindowTitle(filePath.mid(filePath.lastIndexOf("/") + 1) + " Info");

    // Make the window unresizable horizontally
    setMinimumWidth(400);
    setMaximumWidth(400);

    // Fix the first column to the width of the longest label
    ui->gridLayout->setColumnStretch(0, 0);

    // Add a horizontal spacer to the right of the icon
    ui->gridLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 2, 1, 1);

    // Set width of the button to the width needed for the text
    ui->changeOpenWithButton->setFixedWidth(ui->changeOpenWithButton->fontMetrics().boundingRect(ui->changeOpenWithButton->text()).width() + 50);


    ui->iconInfo->setStyleSheet("QLabel { border: 1px solid grey; }");
    ui->iconInfo->setFixedSize(128, 128);
    ui->iconInfo->setAlignment(Qt::AlignCenter);
    // Make the icon selectable
    ui->iconInfo->setFocusPolicy(Qt::ClickFocus);

    // Install an event filter so that we can react to any clicks, on the icon or elsewhere
    this->installEventFilter(this);
    ui->iconInfo->installEventFilter(this);

    // Make it so that one can copy and paste the icon
    ui->iconInfo->setContextMenuPolicy(Qt::ActionsContextMenu);
    QAction *copyAction = new QAction(tr("Copy"), this);
    copyAction->setShortcut(Qt::CTRL + Qt::Key_C);
    connect(copyAction, &QAction::triggered, this, &InfoDialog::copyIcon);
    ui->iconInfo->addAction(copyAction);
    QAction *pasteAction = new QAction(tr("Paste"), this);
    pasteAction->setShortcut(Qt::CTRL + Qt::Key_V);
    connect(pasteAction, &QAction::triggered, this, &InfoDialog::pasteIcon);
    ui->iconInfo->addAction(pasteAction);

    setupInformation();

    // Destroy the dialog when it is closed
    setAttribute(Qt::WA_DeleteOnClose);

    QAction *closeAction = new QAction(this);
    closeAction->setShortcut(Qt::CTRL + Qt::Key_W);
    connect(closeAction, &QAction::triggered, this, &InfoDialog::close);
    addAction(closeAction);

    connect(ui->executableCheckBox, &QCheckBox::clicked, this, &InfoDialog::setExecutable);

    connect(ui->changeOpenWithButton, &QPushButton::clicked, this, &InfoDialog::changeOpenWith);

    bool iconClickedHandled = false;
}

InfoDialog::~InfoDialog()
{
    instances.remove(filePath);
    delete ui;
}

void InfoDialog::setupInformation()
{

    QFile file(filePath);
    if (file.exists() && file.permissions() & QFile::WriteOwner) {
        isEditable = true;
    } else {
        isEditable = false;
    }

    QIcon icon = QIcon::fromTheme("unknown");
    ui->iconInfo->setPixmap(icon.pixmap(128, 128));

    // Get the icon from CustomFileIconProvider
    CustomFileIconProvider *iconProvider = new CustomFileIconProvider();

    // CustomFileSystemModel *model = new CustomFileSystemModel();
    // Parent directory of filePath
    QString parentDir = filePath.mid(0, filePath.lastIndexOf("/"));
    // Need to set a model so that we can get the proper document icon.
    // Since CustomFileIconProvider now needs a QAbstractProxyModel, we need to create one...
    // TODO: Make this less convoluted, e.g,. by making CustomFileIconProvider take a QAbstractItemModel
    //  in addition to a QAbstractProxyModel
    CustomFileSystemModel *sourceModel = new CustomFileSystemModel();
    sourceModel->setRootPath(parentDir);
    QSortFilterProxyModel *model = new QSortFilterProxyModel();
    qDebug() << "Still alive";
    model->setSourceModel(sourceModel);
    qDebug() << "Alive no more";
    iconProvider->setModel(model);

    QIcon i = iconProvider->icon(fileInfo);
    if (!i.isNull()) {
        ui->iconInfo->setPixmap(i.pixmap(128, 128));
    }
    openWith = sourceModel->openWith(filePath); // Used below
    delete model;
    delete sourceModel;
    delete iconProvider;

    ui->pathInfo->setText(filePath);
    ui->pathInfo->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

    // Convert the size into a human-readable format
    QString sizeString;
    sizeString = convertToHumanReadableSize(fileInfo.size());
    ui->sizeInfo->setText(sizeString + " (" + QString::number(fileInfo.size()) + " bytes)");

    ui->createdInfo->setText(fileInfo.created().toString(Qt::DefaultLocaleLongDate));

    ui->modifiedInfo->setText(fileInfo.lastModified().toString(Qt::DefaultLocaleLongDate));

    updatePermissions();

    ui->typeInfo->setText(tr("Unknown"));

    QMimeDatabase *db = new QMimeDatabase();
    QMimeType mime = db->mimeTypeForFile(filePath);
    // Get the description of the MIME type
    QString description = mime.comment();
    if (!description.isEmpty()) {
        ui->typeInfo->setText(description);
    }

    // Check if it is a bundle and if it is, show its type
    // Check if the item is an application bundle and return the icon
    ApplicationBundle *b = new ApplicationBundle(filePath);
    if (b->isValid()) {
        ui->openWithInfo->setText("launch");
        ui->typeInfo->setText(b->typeName());
        if (b->type() == ApplicationBundle::Type::AppBundle || b->type() == ApplicationBundle::Type::AppDir) {
            ui->executableCheckBox->setChecked(true);
        }
    }
    delete b;

    if (!mime.name().isEmpty()) {
        ui->typeInfo->setText(ui->typeInfo->text() + " (" + mime.name() + ")");
    }
    delete db;

    ui->openWithInfo->setText(openWith);
    if (openWith.isEmpty() || ! isEditable) {
        ui->changeOpenWithButton->setEnabled(false);
    } else {
        ui->changeOpenWithButton->setEnabled(true);
    }

    // If it is a symlink, say "Symbolic link to <target>"
    if (fileInfo.isSymLink()) {
        ui->typeInfo->setText(tr("Symbolic link to ") + fileInfo.symLinkTarget());
        ui->openWithInfo->setText("open");
        ui->changeOpenWithButton->setEnabled(false);
    }

    // If it is a mountpoint, show the filesystem type
    if (Mountpoints::isMountpoint(filePath)) {
        QStorageInfo info(filePath);

        QString fileSystemType = info.fileSystemType();
        QString device = info.device();
        QString mountpoint = info.rootPath();

        // Disk usage
        qint64 usedSpace = info.bytesTotal() - info.bytesFree();
        qint64 freeSpace = info.bytesFree();
        qint64 totalSpace = info.bytesTotal();
        QString usedSpaceString = convertToHumanReadableSize(usedSpace);
        QString freeSpaceString = convertToHumanReadableSize(freeSpace);
        QString totalSpaceString = convertToHumanReadableSize(totalSpace);
        QString percentageUsed = QString::number(usedSpace * 100 / totalSpace) + "%";
        QString spaceString = percentageUsed + " " + tr("Full") + " (" + tr("Used: ") + usedSpaceString + ", " + tr("Free: ") + freeSpaceString + ", " + tr("Total: ") + totalSpaceString + ")";
        ui->sizeInfo->setText(spaceString);
        // TODO: Graphical representation of disk usage

        // If it is "fuse", show the actual filesystem type
        if (fileSystemType == "fusefs") {
            QString mountpointToBeFound = fileInfo.absoluteFilePath();
            if (fileInfo.isSymLink()) {
                mountpointToBeFound = fileInfo.symLinkTarget();
            }
            qDebug() << "It is a fuse filesystem, so we need to find out the actual filesystem type";
            if (QSysInfo::kernelType() == "freebsd" && QFile::exists("/var/log/automount.log")) {
                qDebug() << "It is FreeBSD and /var/log/automount.log exists, so we use it";
                // FIXME: Is there a better way to do this?
                QString output = "";
                QFile file("/var/log/automount.log");
                if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                    QTextStream in(&file);
                    while (!in.atEnd()) {
                        QString line = in.readLine();
                        if (line.contains(mountpointToBeFound) && line.contains("mount OK")) {
                            output = line;
                        }
                    }
                    file.close();
                }
                qDebug() << "output:" << output;
                if (!output.isEmpty()) {
                    // Line format is like this:
                    // 2023-28-28 22:14:34 /dev/da0s1: mount OK: 'mount.exfat -o uid=0 -o gid=0 -o umask=002 -o noatime /dev/da0s1 /media/Ventoy'
                    // Parse the line, get the filesystem type, the device and the mountpoint
                    QString partEnclosedInSingleQuotes = output.split("'")[1];
                    // qDebug() << "partEnclosedInSingleQuotes:" << partEnclosedInSingleQuotes;
                    fileSystemType = partEnclosedInSingleQuotes.split(" ")[0];
                    // Remove "mount."
                    fileSystemType = fileSystemType.replace("mount.", "");
                    qDebug() << "fileSystemType:" << fileSystemType;
                    // Device is the second last word
                    device = partEnclosedInSingleQuotes.split(" ")[
                            partEnclosedInSingleQuotes.split(" ").length() - 2];
                    qDebug() << "device:" << device;
                    // Mountpoint is the last word
                    mountpoint = partEnclosedInSingleQuotes.split(" ")[
                            partEnclosedInSingleQuotes.split(" ").length() - 1];
                    qDebug() << "mountpoint:" << mountpoint;
                }
            }
        }

        ui->typeInfo->setText(tr("%1 on %2 mounted at %3").arg(fileSystemType, device).arg(mountpoint));
        ui->openWithInfo->setText("open");
        ui->changeOpenWithButton->setEnabled(false);
    }

}

void InfoDialog::changeOpenWith()
{
    // Print the name of the called function
    qDebug() << Q_FUNC_INFO;

    // Use the "open" command to open the file
    QProcess process;
    process.setProgram("open");
    process.setArguments({ "--chooser", filePath });
    process.startDetached();
}

void InfoDialog::updatePermissions()
{
    QFile file(filePath);

    if (!file.exists())
    {
        qDebug() << "File does not exist.";
        return;
    }

    QFile::Permissions permissions = file.permissions();

    QString permissionsString = getPermissionsString(permissions);

    ui->permissionsInfo->setText(permissionsString);

    // Set permissions checkbox to true, false, or tristate
    if (permissions & QFile::ExeOwner && permissions & QFile::ExeGroup && permissions & QFile::ExeOther) {
        ui->executableCheckBox->setCheckState(Qt::Checked);
        ui->openWithInfo->setText("launch");
    } else if (!(permissions & QFile::ExeOwner) && !(permissions & QFile::ExeGroup) && !(permissions & QFile::ExeOther)) {
        ui->executableCheckBox->setCheckState(Qt::Unchecked);
        ui->openWithInfo->setText(openWith);
    } else {
        ui->executableCheckBox->setCheckState(Qt::PartiallyChecked);
        ui->openWithInfo->setText(openWith);
    }

    if (fileInfo.isDir()) {
        // For directories, we don't check the checkbox and we disable it
        ui->executableCheckBox->setCheckState(Qt::Unchecked);
        ui->executableCheckBox->setEnabled(false);
    } else if (isEditable) {
        // For files, enable the checkbox if the file is editable
        ui->executableCheckBox->setEnabled(true);
    } else {
        // For files, disable the checkbox if the file is not editable
        ui->executableCheckBox->setEnabled(false);
    }
}

void InfoDialog::setExecutable()
{
    QFile file(filePath);

    if (!file.exists())
    {
        qDebug() << "File does not exist.";
        return;
    }

    QFile::Permissions permissions = file.permissions();
    QProcess process;
    if (ui->executableCheckBox->checkState() == Qt::Checked) {
        process.start("chmod", QStringList() << "+x" << filePath);
        process.waitForFinished();
    } else {
        process.start("chmod", QStringList() << "-x" << filePath);
        process.waitForFinished();
    }
    if (process.exitCode() != 0) {
        QMessageBox::warning(this, tr("Error"), tr("Error setting permissions."));
        updatePermissions();
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

void InfoDialog::fileChanged(const QString &path)
{
    qDebug() << "File changed: " << path;
    if (path == filePath)
    {
        fileInfo.refresh(); // Refresh the file information
        setupInformation(); // Update displayed information
        updatePermissions();
    }

    // The MainWindow (currently) only watches the directory, not the items inside it;
    // so we touch the parent directory to trigger a refresh. This does result in
    // an updated icon when the permissions have been changed
    QProcess process;
    qDebug() << "Touching parent directory: " << fileInfo.dir().path();
    process.start("touch", QStringList() << fileInfo.dir().path());
    process.waitForFinished();
}
void InfoDialog::copyIcon()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setPixmap(*ui->iconInfo->pixmap());
}

void InfoDialog::pasteIcon()
{
    // Try to construct a QIcon from the clipboard
    QClipboard *clipboard = QApplication::clipboard();
    QIcon icon = QIcon(clipboard->pixmap());
    if (!icon.isNull()) {
        ui->iconInfo->setPixmap(icon.pixmap(128, 128));
        // Info dialog saying that storing the icon in the file is not implemented yet
        QMessageBox::information(this, tr("Not implemented"), tr("Storing the icon in the file is not implemented yet."));
    } else {
        qDebug() << "Could not construct icon from clipboard.";
    }
}

// Event filter for the icon label to change the border when clicked
bool InfoDialog::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::MouseButtonPress) {
        if (obj == ui->iconInfo && isEditable) {
            ui->iconInfo->setStyleSheet("QLabel { border: 2px solid black; }");
            labelActive = true;
            iconClickedHandled = true;
        } else {
            if (!iconClickedHandled) {
                ui->iconInfo->setStyleSheet("QLabel { border: 1px solid grey; }");
                labelActive = false;
            } else {
                iconClickedHandled = false;
            }
        }
    }

    // Call the base event filter to ensure proper event handling
    return QObject::eventFilter(obj, event);
}

QString InfoDialog::convertToHumanReadableSize(qint64 size) {
    QString sizeString;
    if (size < 1024) {
        sizeString = QString::number(size) + " B";
    } else if (size < 1024 * 1024) {
        sizeString = QString::number(size / 1024.0, 'f', 2) + " KB";
    } else if (size < 1024 * 1024 * 1024) {
        sizeString = QString::number(size / (1024.0 * 1024.0), 'f', 2) + " MB";
    } else {
        sizeString = QString::number(size / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
    }
    return sizeString;
}