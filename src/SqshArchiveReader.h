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

/*
 * Example usage (in the real world, should use "this" instead of deleting the reader and elfBinary objects):

    const char* sqsh_file = "/home/user/Desktop/appimagetool-730-x86_64.AppImage";
    qDebug() << "sqsh_file" << sqsh_file;

    qint64 offset = ElfSizeCalculator::calculateElfSize(sqsh_file);
    qDebug() << "offset" << offset;
    SqshArchiveReader *reader = new SqshArchiveReader(offset);

    QByteArray fileData = reader->readFileFromArchive(sqsh_file, ".DirIcon");
    QIcon icon;
    icon.addPixmap(QPixmap::fromImage(QImage::fromData(fileData)), QIcon::Normal, QIcon::Off);
    qDebug() << icon.availableSizes();

    QByteArray desktopData = reader->readFileFromArchive(sqsh_file, "appimagetool.desktop");
    QString desktopFile = QString::fromUtf8(desktopData);
    QStringList desktopEntries = desktopFile.split("\n");
    desktopEntries.removeAll("");
    qDebug() << desktopEntries;

    QStringList results = reader->readSqshArchive(sqsh_file);
    qDebug() << results;

    delete reader;
    exit(0);

 (QSize(128, 128))
 ("[Desktop Entry]", "Type=Application", "Name=appimagetool", "Exec=appimagetool", "Comment=Tool to generate AppImages from AppDirs", "Icon=appimagetool", "Categories=Development;", "Terminal=true")
 (".DirIcon", "AppRun", "appimagetool.desktop", "appimagetool.png", "usr")

 */

#ifndef SQSHARCHIVEREADER_H
#define SQSHARCHIVEREADER_H

#include <QObject>
#include <QString>
#include <QByteArray>

/**
 * @brief The SqshArchiveReader class provides functionality to read files from a SquashFS archive.
 *
 * The class allows reading the contents of a SquashFS archive and individual files from it.
 * It provides methods to list all the files in the archive and read the contents of specific files.
 */
class SqshArchiveReader : public QObject {
Q_OBJECT

public:
    /**
     * @brief Constructs a SqshArchiveReader object with the specified archive offset.
     * @param archive_offset The offset at which the SquashFS archive starts in the source file (default is 0).
     * @param parent The parent QObject (default is nullptr).
     */
    explicit SqshArchiveReader(uint64_t archive_offset = 0, QObject* parent = nullptr);

    /**
     * @brief Reads and returns the list of files present in the SquashFS archive.
     * @param sqsh_file The path to the SquashFS archive file.
     * @return QStringList containing the names of all the files in the archive.
     */
    QStringList readSqshArchive(const QString& sqsh_file);

    /**
     * @brief Reads the contents of a specific file from the SquashFS archive.
     * @param sqsh_file The path to the SquashFS archive file.
     * @param file_path The path of the file to read from the archive.
     * @return QByteArray containing the data of the requested file.
     */
    QByteArray readFileFromArchive(const QString& sqsh_file, const QString& file_path);

private:
    uint64_t archive_offset_; ///< The offset at which the SquashFS archive starts in the source file.
};

#endif // SQSHARCHIVEREADER_H
