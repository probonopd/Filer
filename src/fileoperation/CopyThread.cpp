#include "CopyThread.h"
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QDirIterator>

CopyThread::CopyThread(const QStringList& fromPaths, const QString& toPath, QObject* parent)
        : QThread(parent), fromPaths(fromPaths), toPath(toPath) {
    connect(this, &CopyThread::cancelCopyRequested, this, &CopyThread::requestInterruption);
}

void CopyThread::run() {
    qint64 totalSize = calculateTotalSize();
    qint64 copiedSize = 0;

    for (const QString& fromPath : fromPaths) {
        QFileInfo fromInfo(fromPath);
        QFileInfo toInfo(toPath);
        QDir toDir(toPath);

        // Check if source is readable
        if (!fromInfo.exists() || !fromInfo.isReadable()) {
            emit error(tr("Source path does not exist or is not accessible."));
            return;
        }

        // Check if source is a directory
        if (toInfo.exists() && !toInfo.isDir()) {
            emit error(tr("Target path must be a directory."));
            return;
        }

        // Check if destination is writable
        if (!toInfo.isWritable()) {
            emit error(tr("Target path is not writable."));
            return;
        }

        // Check if destination is a subdirectory of the source
        if (toInfo.absoluteFilePath().startsWith(fromInfo.absoluteFilePath())) {
            emit error(tr("Target path is a subdirectory of the source."));
            return;
        }

        // Check if destination already exists
        QString toPath = toInfo.absoluteFilePath() + QDir::separator() + fromInfo.fileName();
        if (QFileInfo(toPath).exists()) {
            emit error(tr("Target already exists at the destination."));
            return;
        }

        if (!toDir.exists() && !toDir.mkpath(".")) {
            qDebug() << "Creating target directory" << toDir.absolutePath();
            emit error(tr("Cannot create the target directory."));
            return;
        }

        // Handle symlinks
        if (fromInfo.isSymLink()) {
            QString symlinkTarget = fromInfo.symLinkTarget();
            if (!QFile::link(symlinkTarget, toPath)) {
                emit error(tr("Failed to copy symbolic link."));
                return;
            }
        }

        QString toSubdir = toDir.filePath(fromInfo.fileName());
        if (fromInfo.isDir()) {
            qDebug() << "Creating target subdirectory" << toSubdir;
            if (!toDir.mkpath(toSubdir)) {
                emit error(tr("Cannot create the target subdirectory."));
                return;
            }
        }

        if (fromInfo.isFile()) {
            QString toFilePath = toInfo.absoluteFilePath() + QDir::separator() + fromInfo.fileName();

            if (QFileInfo(toFilePath).exists()) {
                emit error(tr("%1 already exists at the destination.").arg(fromInfo.fileName()));
                return;
            }

            QFile sourceFile(fromPath);
            QFile targetFile(toFilePath);

            if (sourceFile.open(QIODevice::ReadOnly) && targetFile.open(QIODevice::WriteOnly)) {
                char buffer[4096];

                while (qint64 bytesRead = sourceFile.read(buffer, sizeof(buffer))) {
                    qint64 bytesWritten = targetFile.write(buffer, bytesRead);
                    copiedSize += bytesWritten;

                    int percentage = static_cast<int>((copiedSize * 100) / totalSize);
                    emit progress(percentage);

                    if (isInterruptionRequested()) {
                        qDebug() << "CopyThread: Interruption requested. Cleaning up and exiting...";
                        targetFile.remove();
                        return;
                    }
                }

                sourceFile.close();
                targetFile.close();
            }
        } else if (fromInfo.isDir()) {
            QDirIterator it(fromPath, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

            while (it.hasNext()) {
                it.next();
                QString relativePath = it.filePath().mid(fromPath.size() + 1);
                QString targetFilePath = toSubdir + QDir::separator() + relativePath;

                if (it.fileInfo().isSymLink()) {
                    // Handle symbolic link within subdirectory
                    QString symlinkTarget = it.fileInfo().symLinkTarget();
                    if (!QFile::link(symlinkTarget, targetFilePath)) {
                        emit error(tr("Failed to copy symbolic link."));
                        return;
                    }
                } else if (QFileInfo(it.fileInfo()).isDir()) {
                    QDir(targetFilePath).mkpath(".");
                } else {
                    QFile sourceFile(it.filePath());
                    QFile targetFile(targetFilePath);

                    if (sourceFile.open(QIODevice::ReadOnly) && targetFile.open(QIODevice::WriteOnly)) {
                        char buffer[4096];

                        while (qint64 bytesRead = sourceFile.read(buffer, sizeof(buffer))) {
                            qint64 bytesWritten = targetFile.write(buffer, bytesRead);
                            copiedSize += bytesWritten;

                            int percentage = static_cast<int>((copiedSize * 100) / totalSize);
                            emit progress(percentage);

                            if (isInterruptionRequested()) {
                                qDebug() << "CopyThread: Interruption requested. Cleaning up and exiting...";
                                targetFile.remove();
                                return;
                            }
                        }

                        sourceFile.close();
                        targetFile.close();
                    }
                }
            }
        }
    }

    emit progress(100);
    emit copyFinished();
}

qint64 CopyThread::calculateTotalSize() {
    qint64 totalSize = 0;

    for (const QString& fromPath : fromPaths) {
        QFileInfo fromInfo(fromPath);
        if (fromInfo.isFile()) {
            totalSize += fromInfo.size();
        } else if (fromInfo.isDir()) {
            QDirIterator it(fromPath, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
            while (it.hasNext()) {
                it.next();
                totalSize += QFileInfo(it.fileInfo()).size();
            }
        }
    }

    return totalSize;
}