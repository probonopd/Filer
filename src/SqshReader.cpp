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

#include "SqshReader.h"
#include <sqsh.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

SqshArchiveReader::SqshArchiveReader(uint64_t archive_offset, QObject* parent)
        : QObject(parent), archive_offset_(archive_offset) {}

void SqshArchiveReader::setArchiveOffset(uint64_t offset) {
    archive_offset_ = offset;
}

QStringList SqshArchiveReader::readSqshArchive(const char* sqsh_file) {
    QStringList names;
    int error_code = 0;
    struct SqshConfig config = {
            .archive_offset = archive_offset_,
            .source_mapper = sqsh_mapper_impl_mmap,
            .source_size = 0,
            .mapper_block_size = 0,
            .mapper_lru_size = 0,
            .compression_lru_size = 0,
    };
    struct SqshArchive *archive = sqsh_archive_new(sqsh_file, &config, &error_code);

    if (error_code != 0) {
        sqsh_perror(error_code, "sqsh_archive_new");
        return names;
    }
    const struct SqshSuperblock *superblock = sqsh_archive_superblock(archive);
    uint64_t inode_root_ref = sqsh_superblock_inode_root_ref(superblock);
    struct SqshInode *inode = sqsh_inode_new(archive, inode_root_ref, &error_code);
    if (error_code != 0) {
        sqsh_perror(error_code, "sqsh_inode_new");
        return names;
    }
    struct SqshDirectoryIterator *iterator = sqsh_directory_iterator_new(inode, &error_code);
    if (error_code != 0) {
        sqsh_perror(error_code, "sqsh_directory_iterator_new");
        return names;
    }
    while (sqsh_directory_iterator_next(iterator) > 0) {
        const char *name = sqsh_directory_iterator_name(iterator);
        size_t size = sqsh_directory_iterator_name_size(iterator);
        // Null-terminate the name before creating the QString
        char* null_terminated_name = (char*)malloc(size + 1);
        memcpy(null_terminated_name, name, size);
        null_terminated_name[size] = '\0';

        QString nameString = QString::fromUtf8(null_terminated_name);
        names.append(nameString);

        free(null_terminated_name);
    }
    sqsh_directory_iterator_free(iterator);
    sqsh_inode_free(inode);
    sqsh_archive_close(archive);

    return names;
}

QByteArray SqshArchiveReader::readFileFromArchive(const char* sqsh_file, const char* file_path) {
    QByteArray data;
    int error_code = 0;
    struct SqshConfig config = {
            .source_mapper = sqsh_mapper_impl_mmap,
            .source_size = 0,
            .mapper_block_size = 0,
            .mapper_lru_size = 0,
            .compression_lru_size = 0,
            .archive_offset = archive_offset_,
    };
    struct SqshArchive *archive = sqsh_archive_new(sqsh_file, &config, &error_code);
    if (error_code != 0) {
        sqsh_perror(error_code, "sqsh_archive_new");
        return data;
    }
    struct SqshInode *inode = sqsh_open(archive, file_path, &error_code);
    if (error_code != 0) {
        sqsh_perror(error_code, "sqsh_open");
        sqsh_archive_close(archive);
        return data;
    }
    struct SqshFileIterator *iterator = sqsh_file_iterator_new(inode, &error_code);
    if (error_code != 0) {
        sqsh_perror(error_code, "sqsh_file_iterator_new");
        sqsh_inode_free(inode);
        sqsh_archive_close(archive);
        return data;
    }
    while (sqsh_file_iterator_next(iterator, SIZE_MAX) > 0) {
        const uint8_t *file_data = sqsh_file_iterator_data(iterator);
        size_t size = sqsh_file_iterator_size(iterator);
        data.append(reinterpret_cast<const char*>(file_data), size);
    }
    sqsh_file_iterator_free(iterator);
    sqsh_inode_free(inode);
    sqsh_archive_close(archive);

    return data;
}
