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

#include "CustomFileSystemModel.h"
#include "ExtendedAttributes.h"
#include <QDebug>
#include "ApplicationBundle.h"
#include <QMimeData>
#include <QUrl>
#include <QMessageBox>
#include "CustomFileIconProvider.h"
#include <QRegExpValidator>
#include "Mountpoints.h"
#include <QApplication>
#include <QProcess>
#include <QTimer>
#include <QDir>

CustomFileSystemModel::CustomFileSystemModel(QObject* parent)
        : QFileSystemModel(parent)
{
    LaunchDB ldb;

    m_IconProvider = new CustomFileIconProvider();
}

CustomFileSystemModel::~CustomFileSystemModel()
{
    delete m_IconProvider;
}

QByteArray CustomFileSystemModel::readExtendedAttribute(const QModelIndex& index, const QString& attributeName) const
{
    if (!index.isValid() || index.column() != 0) {
        return QByteArray(); // Invalid index or not pointing to a file entry.
    }

    QString filePath = QFileSystemModel::filePath(index);
    QByteArray attributeValue;

    ExtendedAttributes ea(filePath);
    attributeValue = ea.read(attributeName);

    return attributeValue;
}

// Returns the open-with extended attribute for the given file, or the default application for the file type if the
// attribute is not set (based on information from the LaunchDB).
QString CustomFileSystemModel::openWith(const QFileInfo& fileInfo) const {
    // NOTE: Would like to do this with an index, but don't have a valid index when this gets called for unknown reasons.
    // So we'll use the QFileInfo instead for now.
    QString filePath = fileInfo.absoluteFilePath();

    // If we already have the attribute, return it
    // Try to find this file in the model
    QModelIndex index = CustomFileSystemModel::index(filePath);
    if (index.isValid() && openWithAttributes.contains(index)) {
        return openWithAttributes[index];
    }

    // Otherwise, get it from the file's extended attributes
    QString attributeValue;
    ExtendedAttributes ea(filePath);
    attributeValue = QString(ea.read("open-with"));

    // If it's empty, get it from the LaunchDB
    if (attributeValue.isEmpty()) {
        // Get it from the LaunchDB
        LaunchDB ldb;
        attributeValue = QString(ldb.applicationForFile(filePath));

    }

    if (index.isValid()) {
        // If we found it, store it in the model for future use
        // qDebug() << "Updating model with open-with attribute for " << filePath << ": " << attributeValue;
        openWithAttributes[index] = attributeValue.toUtf8();
    } else {
        // qDebug() << "Did not find " << filePath << " in the model. FIXME: Find the reason why this happens. Working around for now.";
        // Once we have solved this condition, we can use an index instead of a QFileInfo for this method.
    }

    return attributeValue;
}

// https://doc.qt.io/qt-5/model-view-programming.html#inserting-dropped-data-into-a-model
// Dropped data is handled by a model's reimplementation of QAbstractItemModel::dropMimeData()
bool CustomFileSystemModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    qDebug() << "CustomFileSystemModel::dropMimeData action" << action;

    // https://doc.qt.io/qt-5/model-view-programming.html#inserting-dropped-data-into-a-model
    // When a drop occurs, the model index corresponding to the parent item will either be valid,
    // indicating that the drop occurred on an item, or it will be invalid,
    // indicating that the drop occurred somewhere in the view that corresponds to top level of the model.
    QModelIndex index = parent;
    QString dropTargetPath;
    if (!index.isValid()) {
        qDebug() << "CustomFileSystemModel::dropMimeData Drop occurred on top level of the model (not on an item)";
        dropTargetPath = rootPath();
        qDebug() << "CustomFileSystemModel::dropMimeData dropTargetPath:" << dropTargetPath;
    } else {
        qDebug() << "CustomFileSystemModel::dropMimeData Drop occurred on an item, TODO: Handle in item delegate";
        qDebug() << "CustomFileSystemModel::dropMimeData index:" << index;

        // Get root index of this model
        QModelIndex rootIndex = this->index(rootPath());
        qDebug() << "CustomFileSystemModel::dropMimeData rootIndex:" << rootIndex;
        // Get the full path of the root index
        QString rootPath = filePath(rootIndex);
        qDebug() << "CustomFileSystemModel::dropMimeData rootPath:" << rootPath;

        // From the model, get the full path of the item that was dropped on
        dropTargetPath = filePath(index);
        qDebug() << "CustomFileSystemModel::dropMimeData dropTargetPath:" << dropTargetPath;

        // Get the full path and compare it; if it is the same, then do as in "drop occurred on top level of the model"
        if (dropTargetPath == rootPath) {
            qDebug() << "CustomFileSystemModel::dropMimeData Drop occurred on an item but it is the same as the root item, TODO: Handle!";
            qDebug() << "XXXXXXXXXXXXXX FIXME XXXXXXXXXXXXXXX";
            // In this case, CustomListView::specialDropEvent() will still get called but it will
            // set the wrong position, so we need to handle this case here.
            // FIXME: Find a solution for this.

        } else {
            qDebug() << "CustomFileSystemModel::dropMimeData Drop occurred on an item, TODO: Handle in item delegate";
            qDebug() << "CustomFileSystemModel::dropMimeData index:" << index;
        }

        // TODO: Handle this case in the item delegate since finding out which item was dropped on is not trivial here
    }

    if (data->hasUrls()) {
        QList<QUrl> urls = data->urls();
        for (int i = 0; i < urls.size(); ++i) {
            if (urls.at(i).isLocalFile()) {
                qDebug() << "CustomFileSystemModel::dropMimeData Dropped file" << urls.at(i).toLocalFile();
            } else {
                qDebug() << "CustomFileSystemModel::dropMimeData Dropped URL" << urls.at(i).toString();
                int success = createBrowserBookmarkFile(data, dropTargetPath);
                if (success) {
                    qDebug() << "CustomFileSystemModel::dropMimeData Successfully created bookmark file";
                } else {
                    QMessageBox::warning(0, 0, tr("Could not create bookmark file"));
                }
            }
        }

        // Emit signals to update the view if needed
        // ...

        return true; // Signal that the drop operation was successful
    }

    return false; // Signal that the drop operation failed
}


//  These functions are used to specify the supported drag and drop actions.

Qt::DropActions CustomFileSystemModel::supportedDropActions() const
{
    qDebug() << "CustomFileSystemModel::supportedDropActions";
    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

Qt::DropActions CustomFileSystemModel::supportedDragActions() const
{
    qDebug() << "CustomFileSystemModel::supportedDragActions";
    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

// https://doc.qt.io/qt-5/model-view-programming.html#enabling-drag-and-drop-for-items
Qt::ItemFlags CustomFileSystemModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QFileSystemModel::flags(index);

    if (index.isValid())
        return Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    else
        return Qt::ItemIsEditable |  Qt::ItemIsDropEnabled | defaultFlags;
}

// https://doc.qt.io/qt-5/model-view-programming.html#inserting-dropped-data-into-a-model
// Models can forbid dropping on certain items,
// or depending on the dropped data, by reimplementing QAbstractItemModel::canDropMimeData().
bool CustomFileSystemModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{

    qDebug() << "CustomFileSystemModel::canDropMimeData";

    if (data->hasUrls()) {
        QList<QUrl> urls = data->urls();
        for (int i = 0; i < urls.size(); ++i) {
            QString filePath = urls.at(i).toLocalFile();
            qDebug() << "CustomFileSystemModel::canDropMimeData Dragged file" << filePath;
        }

        // Emit signals to update the view if needed
        // ...

        return true; // Signal that the drop operation was successful
    }

    return false; // Signal that the drop operation failed
}

QString CustomFileSystemModel::makeFilenameSafe(const QString& input) const
{
    QString output = input;
    // Convert the string to lowercase
    output = output.toLower();
    // Replace all non-alphanumeric characters with a hyphen
    output = output.replace(QRegExp("[^a-z0-9]"), "-");
    // Remove any leading or trailing hyphens
    output = output.replace(QRegExp("^-"), "");
    output = output.replace(QRegExp("-$"), "");
    return output;
}

bool CustomFileSystemModel::createBrowserBookmarkFile(const QMimeData *data, QString dropTargetPath) const {
    qDebug() << "CustomFileSystemModel::createBrowserBookmarkFile" << dropTargetPath;
    QString titleText = "Link";
    if(data->hasText()) {
        titleText = data->text();
    }

    QFile file(dropTargetPath + "/" + makeFilenameSafe(titleText) + ".html");
    qDebug() << "CustomFileSystemModel::createBrowserBookmarkFile" << file.fileName();
    // Write a simple HTML file that redirects to the URL
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << "<!DOCTYPE html>\n";
        stream << "<html>\n";
        stream << "<head>\n";
        stream << "<meta http-equiv=\"refresh\" content=\"0; url=" << data->urls().first().toString(QUrl::FullyEncoded) << "\">\n";
        stream << "</head>\n";
        stream << "<body>\n";
        stream << "</body>\n";
        stream << "</html>\n";
        file.close();
        return true;
    } else {
        return false;
    }
    return false;
}

void CustomFileSystemModel::setPositionForIndex(const QPoint& position, const QModelIndex& index) const {
    // qDebug() << "CustomFileSystemModel::setPositionForIndex";

    QString itemPath = fileInfo(index).absoluteFilePath();
    // qDebug() << "Updating model with coordinates for " << itemPath << ": " << position;
    iconCoordinates[index] = position;

    // Write extended attribute; this is too costly to do here, since setPositionForIndex is called
    // all the time when the window is resized. So we do it in persistItemPositions instead, and only
    // when the user closes the window.
    // ExtendedAttributes *ea = new ExtendedAttributes(itemPath);
    // QString coordinates = QString::number(position.x()) + "," + QString::number(position.y());
    // ea->write("coordinates", coordinates.toUtf8());
    // delete ea;

}

void CustomFileSystemModel::persistItemPositions() const {
    qDebug() << "CustomFileSystemModel::persistItemPositions";

    // Iterate over all iconCoordinates and write them to extended attributes
    for (QModelIndex index : iconCoordinates.keys()) {
            QString itemPath = fileInfo(index).absoluteFilePath();
            qDebug() << "Writing coordinates for " << itemPath << ": " << iconCoordinates[index];
            // Write extended attribute
            ExtendedAttributes *ea = new ExtendedAttributes(itemPath);
            QString coordinates = QString::number(iconCoordinates[index].x()) + "," + QString::number(iconCoordinates[index].y());
            ea->write("coordinates", coordinates.toUtf8());
            delete ea;
    }
}

QPoint& CustomFileSystemModel::getPositionForIndex(const QModelIndex& index) const {
    // qDebug() << "CustomFileSystemModel::getPositionForIndex";

    if (index.isValid() && iconCoordinates.contains(index)) {
        return iconCoordinates[index];
    }

    // Otherwise, get them from extended attributes
    QPoint *coords = new QPoint(-1, -1); // Invalid coordinates; we'll use this to indicate that we didn't find any
    // FIXME: Destroy coords later, otherwise we'll leak memory?

    QString filePath = this->filePath(index);
    ExtendedAttributes ea(filePath);
    QString coordinates = ea.read("coordinates");
    if (!coordinates.isEmpty()) {
        qDebug() << "Read coordinates from extended attributes: " << coordinates;
        QStringList coordinatesList = coordinates.split(",");
        int x = coordinatesList.at(0).toInt();
        int y = coordinatesList.at(1).toInt();
        coords = new QPoint(x, y);
        if (index.isValid()) {
            // If we found it, store it in the model for future use
            // qDebug() << "Updating model with coordinates for " << filePath << ": " << coords;
            iconCoordinates[index] = *coords;
        }
    }

    return *coords;
}

QVariant CustomFileSystemModel::data(const QModelIndex& index, int role) const
{

    QString roleName = roleNames().value(role);
    // qDebug() << "CustomFileSystemModel::data " << index << roleName;

    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        if (index.column() == 0) {
            // If it is an application, use its name
            bool isApplication = data(index, IsApplicationRole).toBool();
            if (isApplication) {
                ApplicationBundle *ab = new ApplicationBundle(filePath(index));
                QString name = ab->name();
                delete ab;
                return name;
            } else {
                // Otherwise, use the filename
                return data(index, Qt::EditRole);
            }

        }
    }

    if (role == Qt::DecorationRole) {
        // TODO: Icon generation can be slow; is there a way to use async/threaded/callbacks
        // so that icons are generated in the background and are shown whenever they are ready?
        if (index.column() == 0) {
            QFileInfo fileInfo = this->fileInfo(index);
            // If openWith, we use m_IconProvider->documentIcon
            QString openWithString = data(index, OpenWithRole).toString();
            if (openWithString != "") {
                return m_IconProvider->documentIcon(fileInfo, openWithString);
            } else {
                return m_IconProvider->icon(fileInfo);
            }
        }
    }

    if (role == OpenWithRole) {

        // Return the cached value if we have it
        if (openWithAttributes.contains(index)) {
            // qDebug() << "CustomFileSystemModel::data OpenWithRole (cached)" << filePath(index);
            return openWithAttributes[index];
        }

        // If we don't have it cached, read the open-with extended attribute
        QString attributeValue;
        ExtendedAttributes ea(filePath(index));
        attributeValue = QString(ea.read("open-with"));

        // If it's empty, get it from the LaunchDB
        if (attributeValue.isEmpty()) {
            // Get it from the LaunchDB
            LaunchDB *ldb = new LaunchDB();
            attributeValue = QString(ldb->applicationForFile(filePath(index)));
            delete ldb;
        }
        openWithAttributes[index] = attributeValue.toUtf8();
        return attributeValue;
    }

    if (role == CanOpenRole) {
        // Return the cached value if we have it
        if (canOpenAttributes.contains(index)) {
            // qDebug() << "CustomFileSystemModel::data CanOpenRole (cached)" << filePath(index);
            return canOpenAttributes[index];
        }

        // If we don't have it cached, read the can-open extended attribute
        QString attributeValue;
        ExtendedAttributes ea(filePath(index));
        attributeValue = QString(ea.read("can-open"));

        canOpenAttributes[index] = attributeValue.toUtf8();
        return attributeValue;
    }

    if (role == IsApplicationRole) {
        if (isApplication.contains(index)) {
            return isApplication[index];
        }
        ApplicationBundle *ab = new ApplicationBundle(filePath(index));
        bool isApplicationBundle = ab->isValid();
        delete ab;
        isApplication[index] = isApplicationBundle;
        return isApplicationBundle;
    }

    if (role == Qt::ToolTipRole) {
        QString tooltipText = filePath(index);
        if (data(index, OpenWithRole).toString() != "") {
            tooltipText += "\nOpen with: " + openWithAttributes[index];
        }

        tooltipText += "\nIs application: " + data(index, IsApplicationRole).toString();

        if (data(index, CanOpenRole).toString() != "") {
            tooltipText += "\nCan open: " + data(index, CanOpenRole).toString();
        }

        // Return the file path as the tooltip
        return tooltipText;
    }

    return QFileSystemModel::data(index, role);
}

// Handle renaming by clicking on the name in the list view or tree view
bool CustomFileSystemModel::setData(const QModelIndex &idx, const QVariant &value, int role) {
    qDebug() << "CustomFileSystemModel::setData " << idx << value << role;

    // Get the current name of the selected item
    const QString currentName = fileName(idx);

    // Prevent the user from using "/" anywhere in the new name
    QRegExpValidator validator(QRegExp("[^/]*"));
    bool ok;

    // Check if the item to be renamed is a mountpoint

    const QString currentPath = filePath(idx);
    QString absoluteFilePath = QFileInfo(currentPath).absoluteFilePath();
    // if absoluteFilePath is a symlink, resolve it
    if (QFileInfo(absoluteFilePath).isSymLink()) {
        absoluteFilePath = QFileInfo(absoluteFilePath).symLinkTarget();
    }
    qDebug() << "absoluteFilePath:" << absoluteFilePath;

    if (Mountpoints::isMountpoint(absoluteFilePath)) {
        // Get the filesystem type using QStorageInfo
        const QString filesystemType = QStorageInfo(absoluteFilePath).fileSystemType();
        qDebug() << "Filesystem type:" << filesystemType;
        QStringList renameableFilesystems = { "ext2", "ext3", "ext4", "reiserfs", "reiser4", "ufs", "vfat", "exfat", "ntfs" };
        // If the filesystem is not in the list of renameable filesystems, disable renaming
        if (!renameableFilesystems.contains(filesystemType)) {
            qDebug() << "Filesystem is not in the list of renameable filesystems";
            return false;
        }
    }

    // Get the new name of the selected item
    const QString newName = value.toString();

    if (newName.isEmpty() || newName == currentName) {
        // The user canceled the dialog or didn't enter a new name
        return false;
    }

    qDebug() << "newName:" << newName;
    qDebug() << "currentName:" << currentName;

    /*
    // If there was an extension, compare the old and the new extension
    // and if they are different, ask the user if they want to continue
    if (currentName.lastIndexOf('.') != -1) {
        if (currentName.right(currentName.length() - currentName.lastIndexOf('.') - 1) != newName.right(newName.length() - newName.lastIndexOf('.') - 1)) {
            // The extensions are different
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, 0, tr("Do you really want to change the file extension?"), QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::No) {
                return;
            }
        }
    }
     */

    if (Mountpoints::isMountpoint(absoluteFilePath)) {

        // TODO: Possibly move everything in this if statement to a separate class,
        // similar to the FileOperationManager class

        // The item to be renamed is a mountpoint, so we need to run:
        // sudo -A -E renamedisk <old name> <new name>

        /*
        // Check if we are on FreeBSD and if we are not, show an error message
        if (QSysInfo::kernelType() != "freebsd") {
            QMessageBox::critical(0, "Filer", "The 'renamedisk' command has only been implemented for FreeBSD yet.");
            return;
        }
        */

        QStringList renamediskBinaryCandidates;

        renamediskBinaryCandidates << qApp->applicationDirPath() + QString("/bin/renamedisk")
                                   << qApp->applicationDirPath() + QString("/../../Resources/renamedisk")
                                   << qApp->applicationDirPath() + QString("/../bin/renamedisk")
                                   << qApp->applicationDirPath() + QString("/renamedisk/renamedisk");

        QString foundBinary;

        for (const QString &renamediskBinaryCandidate : renamediskBinaryCandidates) {
            if (QFile::exists(renamediskBinaryCandidate) && QFileInfo(renamediskBinaryCandidate).isExecutable()) {
                foundBinary = renamediskBinaryCandidate;
                break;
            }
        }

        if (foundBinary.isEmpty()) {
            // Not found
            QMessageBox::critical(0, "Filer", "The 'renamedisk' command is missing. It should have been shipped with this application.");
            return false;
        }

        QString oldName = currentPath.split("/").last();
        QProcess *p = new QProcess(this);
        // TODO: Check if we need sudo at all for this kind of filesystem; e.g., if it's a FAT32 filesystem
        // then we don't need sudo
        p->setProgram("sudo");
        p->setArguments({ "-A", "-E", foundBinary, absoluteFilePath, newName });
        qDebug() << p->program() << "'" + p->arguments().join("' '") + "'";
        p->start();
        p->waitForFinished();
        qDebug() << "renamedisk exit code:" << p->exitCode();
        if (p->exitCode() != 0) {
            QStringList errorLines = QString(p->readAllStandardError()).split("\n");
            for (const QString &errorLine : errorLines) {
                qCritical() << errorLine;
            }
            // QMessageBox::critical(this, tr("Error"), tr("Could not rename %1 to %2").arg(oldName).arg(newName));
            return false;
        } else {
            qDebug() << "Renamed" << currentPath << "to" << newName;
            return true;
            // In 0 ms, touch the parent directory of the renamed mountpoint
            // to make the view update itself
            QTimer::singleShot(0, this, [=]() {
                QFileInfo parentDir(currentPath);
                QProcess::execute("touch", { parentDir.dir().absolutePath() });
            });

            // The view will automatically update itself; works
        }
        return false;
    }

    // Rename the selected item in the file system
    const QString newPath = currentPath.left(currentPath.lastIndexOf("/") + 1) + newName;
    if(!QFile::rename(currentPath, newPath)) {
        // QMessageBox::critical(this, tr("Error"), tr("Could not rename %1 to %2").arg(currentPath).arg(newPath));
        return false;
    } else {
        qDebug() << "Renamed" << currentPath << "to" << newPath;
        // In 0 ms, touch the parent directory of the renamed file
        // to make the view update itself
        QTimer::singleShot(0, this, [=]() {
            QFileInfo parentDir(currentPath);
            QProcess::execute("touch", { parentDir.dir().absolutePath() });
        });
        // The view will automatically update itself; works
        return true;
    }


    return false;
}