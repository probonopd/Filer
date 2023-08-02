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
   const char* sqsh_file = "/tmp/appimagetool-x86_64.AppImage";

    ElfBinary* elfBinary = new ElfBinary();
    qint64 offset = elfBinary->getElfSize(sqsh_file);
    SqshArchiveReader *reader = new SqshArchiveReader(offset);

    QStringList results = reader->readSqshArchive(sqsh_file);
    qDebug() << results;

    QByteArray fileData = reader->readFileFromArchive(sqsh_file, ".DirIcon");
    QIcon icon;
    icon.addPixmap(QPixmap::fromImage(QImage::fromData(fileData)), QIcon::Normal, QIcon::Off);
    qDebug() << icon.availableSizes();

    QByteArray desktopData = reader->readFileFromArchive(sqsh_file, "appimagetool.desktop");
    QString desktopFile = QString::fromUtf8(desktopData);
    QStringList desktopEntries = desktopFile.split("\n");
    desktopEntries.removeAll("");
    qDebug() << desktopEntries;

    delete reader;
    delete elfBinary;

 (".DirIcon", "AppRun", "appimagetool.desktop", "appimagetool.png", "usr")
 (QSize(128, 128))
 ("[Desktop Entry]", "Type=Application", "Name=appimagetool", "Exec=appimagetool", "Comment=Tool to generate AppImages from AppDirs", "Icon=appimagetool", "Categories=Development;", "Terminal=true")

 */

#ifndef SQSHARCHIVEREADER_H
#define SQSHARCHIVEREADER_H

#include <QObject>
#include <QString>
#include <QByteArray>

class SqshArchiveReader : public QObject {
Q_OBJECT
public:
    explicit SqshArchiveReader(uint64_t archive_offset = 0, QObject* parent = nullptr);

    void setArchiveOffset(uint64_t offset);
    QStringList readSqshArchive(const char* sqsh_file);
    QByteArray readFileFromArchive(const char* sqsh_file, const char* file_path);

private:
    uint64_t archive_offset_;
};

#endif // SQSHARCHIVEREADER_H
