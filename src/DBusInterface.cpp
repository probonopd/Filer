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
    QMessageBox::warning(0, 0, "ShowFolders is not yet implemented");
    qDebug() << "ShowFolders" << uriList << startUpId;
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
    QMessageBox::warning(0, 0, "ShowItemProperties is not yet implemented");
    qDebug() << "ShowItemProperties" << uriList << startUpId;
}

void DBusInterface::SortOrderForUrl(const QString &url, QString &role, QString &order)
{
    QMessageBox::warning(0, 0, "SortOrderForUrl is not yet implemented");
    qDebug() << "SortOrderForUrl" << url << role << order;
}