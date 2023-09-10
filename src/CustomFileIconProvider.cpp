#include "CustomFileIconProvider.h"
#include "ApplicationBundle.h"

#include "CombinedIconCreator.h"
#include "ExtendedAttributes.h"

#include <QDebug>
#include <QProcess>
#include <QFile>
#include <QIcon>
#include <QPainter>
#include <QApplication>
#include <QThread>
#include <QImageReader>
#include "AppGlobals.h"
#include "TrashHandler.h"
#include "Mountpoints.h"

#include "Executable.h"

CustomFileIconProvider::CustomFileIconProvider()
{
    m_iconCreator = new CombinedIconCreator();
}

CustomFileIconProvider::~CustomFileIconProvider()
{
    delete m_iconCreator;
}

QIcon CustomFileIconProvider::userIcon(const QFileInfo &info) const {
    // Try to read the "user-icon" extended attribute
    ExtendedAttributes *ea = new ExtendedAttributes(info.absoluteFilePath());
    QString base64IconData = ea->read("user-icon");
    delete ea;
    if (!base64IconData.isEmpty()) {
        qDebug() << "Found user-icon extended attribute";
        // qDebug() << "base64IconData: " << base64IconData;
        QByteArray iconData = QByteArray::fromBase64(base64IconData.toUtf8());
        QImage image;
        image.loadFromData(iconData);
        return (QIcon(QPixmap::fromImage(image)));
    } else {
        return (QIcon());
    }
}

QIcon CustomFileIconProvider::icon(const QFileInfo &info) const
{
    // If the user has set a custom icon, return it
    QIcon icon = userIcon(info);
    if (!icon.isNull()) {
        return (icon);
    }

    // Check if the item is an application bundle and return the icon
    ApplicationBundle *app = new ApplicationBundle(info.absoluteFilePath());
    if (app->isValid()) {
        return (app->icon());
    }
    delete app;

    // If ~/Desktop, ~/Documents, ~/Downloads, ~/Music, ~/Pictures, or ~/Videos, show the respective icon
    // from the current icon theme
    QString filePath = info.absoluteFilePath();
    if (filePath == QDir::homePath() + "/Desktop") {
        return (QIcon::fromTheme("user-desktop"));
    } else if (filePath == QDir::homePath() + "/Documents") {
        return (QIcon::fromTheme("folder-documents"));
    } else if (filePath == QDir::homePath() + "/Downloads") {
        return (QIcon::fromTheme("folder-download"));
    } else if (filePath == QDir::homePath() + "/Music") {
        return (QIcon::fromTheme("folder-music"));
    } else if (filePath == QDir::homePath() + "/Pictures") {
        return (QIcon::fromTheme("folder-pictures"));
    } else if (filePath == QDir::homePath() + "/Videos") {
        return (QIcon::fromTheme("folder-videos"));
    } else if (filePath == QDir::homePath()) {
        return (QIcon::fromTheme("user-home"));
    }

    // Volume icons
    // How many directories deep is AppGlobals::mediaPath?
    int mediaPathDepth = AppGlobals::mediaPath.count("/");

    // Resolve symlinks
    QString absoluteFilePathWithSymLinksResolved = info.absoluteFilePath();
    if (info.isSymLink()) {
        absoluteFilePathWithSymLinksResolved = info.symLinkTarget();
    }

    // How many directories deep is the current file?
    int fileDepth = absoluteFilePathWithSymLinksResolved.count("/");

    // Is the current file in a subdirectory of AppGlobals::mediaPath but not in a sub-subdirectory?
    bool isMediaPath = (absoluteFilePathWithSymLinksResolved.startsWith(AppGlobals::mediaPath) && (fileDepth == mediaPathDepth + 1));

    // Is the current file (symlinks unresolved) in ~/Desktop or a subdirectory of /media?
    // Only then we want to show the drive icons. Otherwise, we get hard disk icons inside the hard disk
    // which is counterintuitive
    QString parentDirPath = info.absoluteFilePath().left(info.absoluteFilePath().lastIndexOf("/"));
    bool isOnDesktopOrInMedia = ((parentDirPath == QDir::homePath() + "/Desktop") || info.absoluteFilePath().startsWith("/media"));

    // If it is a directory and the symlink target is a mount point, then we want to show the drive icon
    if (absoluteFilePathWithSymLinksResolved == "/" || isOnDesktopOrInMedia && info.isDir() && isMediaPath || isOnDesktopOrInMedia && info.isDir() && Mountpoints::isMountpoint(absoluteFilePathWithSymLinksResolved)) {
        // Using Qt, get the device node of the mount point
        // and then use the device node to get the icon
        // qDebug() << "Mount point: " << info.absoluteFilePath();
        QStorageInfo storageInfo(info.absoluteFilePath());
        QString deviceNode = storageInfo.device();
        qDebug() << "Device node: " << deviceNode;

        // If it is not mounted, then show the folder icon
        QStringList mountPoints;
        for (const QStorageInfo &storage : QStorageInfo::mountedVolumes()) {
            mountPoints << storage.rootPath();
        }
        if (! mountPoints.contains(absoluteFilePathWithSymLinksResolved)) {
            return (QIcon::fromTheme("folder"));
        }

        // Set the icon depending on the file system type; unlike device nodes,
        // this also works for mounted disk images
        QString fileSystemType = storageInfo.fileSystemType();
        qDebug() << "File system type: " << fileSystemType;
        if (fileSystemType == "iso9660" | fileSystemType == "udf" | fileSystemType == "cd9660") {
            return (QIcon::fromTheme("media-optical"));
        }

        // Set the icon depending on the device node
        if (deviceNode.startsWith("/dev/da")){
            return (QIcon::fromTheme("drive-removable-media"));
        } else if (deviceNode.startsWith("/dev/sr") || deviceNode.startsWith("/dev/cd")) {
            return (QIcon::fromTheme("media-optical"));
        } else {
            return (QIcon::fromTheme("drive-harddisk"));
        }
    }

    // If it is not a bundle but a directory, then we want to show the folder icon
    if (info.isDir()) {
        // Check if it is the Trash
        // Resolve symlinks
        QString absoluteFilePathWithSymLinksResolved = info.absoluteFilePath();
        if (info.isSymLink()) {
            absoluteFilePathWithSymLinksResolved = info.symLinkTarget();
        }
        if (absoluteFilePathWithSymLinksResolved == TrashHandler::getTrashPath()) {
            // Check if there are files inside the Trash using QDir::isEmpty()
            if (TrashHandler::isEmpty()) {
                return (QIcon::fromTheme("user-trash"));
            } else {
                return (QIcon::fromTheme("user-trash-full"));
            }
        }
        // If it is lacking permissions, then we want to show the locked folder icon; TODO: Use emblem instead?
        if (!QFileInfo(info.absoluteFilePath()).isReadable() || !QFileInfo(info.absoluteFilePath()).isExecutable()) {
            // Try to get folder-locked icon from the current theme,
            // fall back to other icons if it is not available
            if (QIcon::hasThemeIcon("folder-locked")) {
                return (QIcon::fromTheme("folder-locked"));
            } else if (QIcon::hasThemeIcon("lock")) {
                return (QIcon::fromTheme("lock"));
            } else {
                return (QIcon::fromTheme("cancel"));
            }
        } else {
            return (QIcon::fromTheme("folder"));
        }
    }

    // If we have no read permissions, show the lock icon; TODO: Use emblem instead?
    if (!QFileInfo(info.absoluteFilePath()).isReadable()) {
        // Try to get lock icon from the current theme,
        // fall back to other icons if it is not available
        if (QIcon::hasThemeIcon("lock")) {
            return (QIcon::fromTheme("lock"));
        } else {
            return (QIcon::fromTheme("cancel"));
        }
    }

    // If it is an .exe file, then we want to show the ICO from the .exe file
    if (info.suffix().compare("exe", Qt::CaseInsensitive) == 0)
    {
        qDebug() << "File extension is .exe: " << info.absoluteFilePath();

        // Call icoextract executable to extract the icon
        QProcess icoExtractProcess;
        QStringList arguments;
        arguments << info.absoluteFilePath() << "/tmp/icoextract.ico";
        icoExtractProcess.start("icoextract", arguments);
        if (!icoExtractProcess.waitForStarted() || !icoExtractProcess.waitForFinished())
        {
            qDebug() << "Failed to run icoextract.";
            return QIcon::fromTheme("application-x-ms-dos-executable");
        }

        // Read the extracted icon
        QFile iconFile("/tmp/icoextract.ico");
        if (!iconFile.open(QIODevice::ReadOnly))
        {
            qDebug() << "Failed to open icon file.";
            return QIcon::fromTheme("application-x-ms-dos-executable");
        }
        QByteArray iconData = iconFile.readAll();
        iconFile.close();
        qDebug() << "Icon data size: " << iconData.size();

        // Load the icon data into a QImage
        QImage iconImage;
        if (iconImage.loadFromData(reinterpret_cast<const uchar*>(iconData.constData()), iconData.size(), "ico"))
        {
            // Create a QIcon from the QImage
            QIcon extractedIcon;
            // Scale to 32x32; FIXME: Extract the best fitting size from the .exe file to begin with
            extractedIcon.addPixmap(QPixmap::fromImage(iconImage.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
            return extractedIcon;
        }
        else
        {
            qDebug() << "Failed to load icon image.";
            return QIcon::fromTheme("application-x-ms-dos-executable");
        }
    }

    // If the file has the executable bit set and is not a directory,
    // then we always want to show the executable icon
    if (info.isExecutable() && !info.isDir()) {
        // Try to load the application icon from the path ./Resources/application.png relative to the application executable path
        // If the icon cannot be loaded, use the default application icon from the icon theme
        QString applicationPath = QApplication::applicationDirPath();
        QIcon applicationIcon;
        QString applicationIconPath = applicationPath + "/Resources/application.png";
        if (QFile::exists(applicationIconPath)) {
            applicationIcon = QIcon(applicationIconPath);
            return (applicationIcon);
        } else {
            // E.g., on fat32, not every file that has the executable bit set is actually an executable
            if(Executable::hasShebangOrIsElf(info.absoluteFilePath())) {
                applicationIcon = QIcon::fromTheme("application-x-executable");
                return (applicationIcon);
            }
        }
    }

    // Handle .DirIcon (AppDir) and volumelcon.icns (Mac)
    QStringList candidates = {info.absoluteFilePath() + "/.DirIcon", info.absoluteFilePath() + "/volumelcon.icns"};
    for (const QString &candidate: candidates) {
        if (QFileInfo(candidate).exists()) {
            // Read the contents of the file and turn it into an icon
            QFile file(candidate);
            return (QIcon(file.readAll()));
        }
    }

    // In all other cases, we want to show the default icon
    return QFileIconProvider::icon(info);

}

QIcon CustomFileIconProvider::documentIcon(const QFileInfo &info, QString openWith) const
{
    // If the user has set a custom icon, return it
    QIcon icon = userIcon(info);
    if (!icon.isNull()) {
        return (icon);
    }

    QFileInfo openWithInfo(openWith);
    ApplicationBundle *app = new ApplicationBundle(openWithInfo.absoluteFilePath());
    if (app->isValid()) {
        // qDebug("Info: %s is a valid application bundle", qPrintable(openWith));
        QIcon applicationIcon = QIcon(app->icon()).pixmap(16, 16);
        if (applicationIcon.isNull()) {
            qDebug("Warning: %s does not have an icon", qPrintable(openWith));
            applicationIcon = QIcon::fromTheme("unknown");
        }
        // return(applicationIcon);
        QIcon combinedIcon = m_iconCreator->createCombinedIcon(applicationIcon);
        return (combinedIcon);
    } else {
        // Return generic application icon from theme
        return (QIcon::fromTheme("document"));
    }
    delete app;
}