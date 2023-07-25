#include "LaunchDB.h"
#include <QDir>
#include <QDebug>

LaunchDB::LaunchDB() {
    db = new QMimeDatabase;
}

LaunchDB::~LaunchDB() {
    delete db;
}

QString LaunchDB::applicationForFile(const QFileInfo &fileInfo) {
    // Check if the file exists
    if (!fileInfo.exists()) {
        return QString(); // Return an empty QString if the file doesn't exist
    }

    // Resolve symlinks and get absolute path
    QString absoluteFilePath = fileInfo.absoluteFilePath();
    // If it is a symlink, then resolve it
    if (fileInfo.isSymLink()) {
        absoluteFilePath = fileInfo.symLinkTarget();
    }

    // Get the MIME type of the file
    QMimeType mimeType = db->mimeTypeForFile(absoluteFilePath);
    qDebug() << "mimeType: " << mimeType.name();

    // If we have "text/x-pdf, see whether ~/.local/share/launch/MIME/text_x-pdf/ exists (just as an example)
    QString mimeDir = QDir::homePath() + "/.local/share/launch/MIME/" + mimeType.name().replace("/", "_");
    if (QDir(mimeDir).exists()) {
        // If there is a default application for the MIME type, then return it
        QString defaultApplication = mimeDir + "/Default";
        if (QFile(defaultApplication).exists()) {
            defaultApplication = QFileInfo(defaultApplication).absoluteFilePath();
            // If it is a symlink, then resolve it
            if (QFileInfo(defaultApplication).isSymLink()) {
                defaultApplication = QFileInfo(defaultApplication).symLinkTarget();
            }
            if (!QFile(defaultApplication).exists()) {
                return QString();
            }
            return defaultApplication;
        }
        // If there is only one application for the MIME type (not counting the default application), then return it
        QStringList applications = QDir(mimeDir).entryList(QDir::Files);
        // applications.removeAll("Default");
        if (applications.size() == 1) {
            QString application = mimeDir + "/" + applications.at(0);
            if (QFile(application).exists()) {
                application = QFileInfo(application).absoluteFilePath();
                // If it is a symlink, then resolve it
                if (QFileInfo(application).isSymLink()) {
                    application = QFileInfo(application).symLinkTarget();
                }
                if (!QFile(application).exists()) {
                    return QString();
                }
                return application;
            }
        }
    }

    // Return the default application for the file
    return QString(); // Return an empty QString if we don't know the default application
}
