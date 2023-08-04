#include "DBusInterface.h"

#include <QDebug>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QMessageBox>
#include <QApplication>
#include <QUrl>
#include <QFileInfo>

#include "FileManagerMainWindow.h"

DBusInterface::DBusInterface()
    : QObject()
{
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/org/freedesktop/FileManager1"),
         this,
         QDBusConnection::ExportScriptableContents | QDBusConnection::ExportAdaptors);
    QDBusConnectionInterface *sessionInterface = QDBusConnection::sessionBus().interface();
    if (sessionInterface) {
        sessionInterface->registerService(QStringLiteral("org.freedesktop.FileManager1"),
                                          QDBusConnectionInterface::QueueService);
    }

    if (!QDBusConnection::sessionBus().interface()) {
        QMessageBox::warning(0, 0, tr("Could not register the FileManager1 D-Bus service"));
    }
}

void DBusInterface::ShowFolders(const QStringList &uriList, const QString &startUpId)
{
    qDebug() << "ShowFolders" << uriList << startUpId;

    for (const QString &fileUrlString : uriList) {
        QUrl fileUrl(fileUrlString);
        QString filePath;

        if (fileUrl.isValid()) {
            filePath = fileUrl.toLocalFile();
        } else {
            if (QFileInfo::exists(fileUrlString)) {
                filePath = fileUrlString;
            } else {
                qDebug() << "Not a URL or a local file path:" << fileUrlString;
                QMessageBox::warning(0, QString(), tr("Not a URL or a local file path: %1").arg(fileUrlString));
                continue;
            }
        }

        FileManagerMainWindow *mainWindow = qobject_cast<FileManagerMainWindow *>(qApp->activeWindow());

        if (!mainWindow->instanceExists(filePath)) {
            mainWindow->openFolderInNewWindow(filePath);
        }

        while (!mainWindow->instanceExists(filePath)) {
            qDebug() << "Waiting for instance of FileManagerMainWindow for" << filePath;
            QApplication::processEvents();
        }

        mainWindow = mainWindow->getInstanceForDirectory(filePath);
    }
}


void DBusInterface::ShowItems(const QStringList &uriList, const QString &startUpId)
{
    qDebug() << "ShowItems" << uriList << startUpId;

    for (QString fileUrlString : uriList) {
        // Convert the URL to a local file path
        QUrl fileUrl(fileUrlString);
        QString filePath = fileUrl.toLocalFile();

        // Get the parent directory
        QFileInfo fileInfo(filePath);
        QString parentDir = fileInfo.absoluteDir().path();
        if (!fileInfo.exists()) {
            QMessageBox::warning(0, 0, tr("File does not exist: %1").arg(filePath));
            continue;
        }

        FileManagerMainWindow* mainWindow = qobject_cast<FileManagerMainWindow*>(qApp->activeWindow());

        if (!mainWindow->instanceExists(parentDir)) {
            mainWindow->openFolderInNewWindow(parentDir);
        }

        // Find the instance of FileManagerMainWindow for parentDir
        // Wait until this succeeds: mainWindow->getInstanceForDirectory(parentDir);
        while (!mainWindow->instanceExists(parentDir)) {
            qDebug() << "Waiting for instance of FileManagerMainWindow for" << parentDir;
            QApplication::processEvents();
        }

        mainWindow = mainWindow->getInstanceForDirectory(parentDir);

        QStringList filePathList = QStringList() << filePath;
        mainWindow->selectItems(filePathList);
    }
}

void DBusInterface::ShowItemProperties(const QStringList &uriList, const QString &startUpId)
{
    QMessageBox::warning(0, 0, "ShowItemProperties is not implemented yet");
    qDebug() << "ShowItemProperties" << uriList << startUpId;
}

void DBusInterface::SortOrderForUrl(const QString &url, QString &role, QString &order)
{
    QMessageBox::warning(0, 0, "SortOrderForUrl is not implemented yet");
    qDebug() << "SortOrderForUrl" << url << role << order;
}