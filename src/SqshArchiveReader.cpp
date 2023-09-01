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

#include "SqshArchiveReader.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqsh.h>

SqshArchiveReader::SqshArchiveReader(uint64_t archive_offset, QObject* parent)
        : QObject(parent), archive_offset_(archive_offset) {}

QStringList SqshArchiveReader::readSqshArchive(const QString& sqsh_file) {
    QByteArray sqsh_file_bytes = sqsh_file.toUtf8();
    const void* sqsh_file_source = sqsh_file_bytes.constData();

    char **cnames = NULL;
    QStringList names;
    int error_code = 0;
    struct SqshConfig config = {
            .archive_offset = archive_offset_,
    };
    struct SqshArchive* archive = sqsh_archive_open(sqsh_file_source, &config, &error_code);
    if (error_code != 0) {
        sqsh_perror(error_code, "sqsh_archive_open");
        sqsh_archive_close(archive);
        goto out;
    }

    cnames = sqsh_easy_directory_list(archive, "/", &error_code);
    if (error_code != 0) {
        sqsh_perror(error_code, "sqsh_easy_directory_list");
        goto out;
    }
    for(int i = 0; cnames[i] != NULL; i++) {
        QString nameString = QString::fromUtf8(cnames[i]);
        names.append(nameString);
    }

out:
    free(cnames);
    sqsh_archive_close(archive);
    return names;
}

QByteArray SqshArchiveReader::readFileFromArchive(const QString& sqsh_file, const QString& file_path) {
    QByteArray sqsh_file_bytes = sqsh_file.toUtf8();
    const void* sqsh_file_source = sqsh_file_bytes.constData();

    QByteArray file_path_bytes = file_path.toUtf8();
    const char* file_path_cstr = file_path_bytes.constData();

    uint8_t *file_data = NULL;
    size_t file_size = 0;
    QByteArray data; // Declare the 'data' variable here
    int error_code = 0;
    struct SqshConfig config = {
            .archive_offset = archive_offset_,
    };
    struct SqshArchive* archive = sqsh_archive_open(sqsh_file_source, &config, &error_code);
    if (error_code != 0) {
        sqsh_perror(error_code, "sqsh_archive_open");
        goto out;
    }

    file_size = sqsh_easy_file_size(archive, file_path_cstr, &error_code);
    if (error_code != 0) {
        sqsh_perror(error_code, "sqsh_easy_file_size");
        goto out;
    }
    file_data = sqsh_easy_file_content(archive, file_path_cstr, &error_code);
    if (error_code != 0) {
        sqsh_perror(error_code, "sqsh_easy_file_content");
        goto out;
    }
    data.append(reinterpret_cast<char*>(file_data), file_size);

out:
    free(file_data);
    sqsh_archive_close(archive);

    return data;
}
