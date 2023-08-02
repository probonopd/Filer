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

#include "ElfBinary.h"

#include <QDataStream>
#include <QDebug>

#define EI_NIDENT 16
#define ELFCLASS64  2
#define ELFCLASS32  1
#define EI_CLASS    4
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2
#define EI_DATA     5

ElfBinary::ElfBinary(QObject *parent) : QObject(parent)
{

}

quint64 ElfBinary::bytesToHostByteOrder(const uchar *bytes, int size)
{
    quint64 result = 0;
    for (int i = 0; i < size; ++i) {
        result |= quint64(bytes[i]) << ((size - 1 - i) * 8);
    }
    return result;
}

qint64 ElfBinary::readElf64(QFile &file)
{
    Elf64_Ehdr ehdr64;
    Elf64_Shdr shdr64;
    qint64 last_shdr_offset;
    qint64 ret;
    qint64 sht_end, last_section_end;

    file.seek(0); // Move to the beginning of the file
    ret = file.read(reinterpret_cast<char*>(&ehdr64), sizeof(ehdr64));
    if (ret < 0 || ret != sizeof(ehdr64)) {
        qWarning() << "Read of ELF header failed.";
        return -1;
    }

    if ((ehdr64.e_ident[EI_DATA] != ELFDATA2LSB) &&
        (ehdr64.e_ident[EI_DATA] != ELFDATA2MSB)) {
        qWarning() << "Unknown ELF data order" << ehdr64.e_ident[EI_DATA];
        return -1;
    }

    sht_end = ehdr64.e_shoff + (ehdr64.e_shentsize * ehdr64.e_shnum);
    last_shdr_offset = ehdr64.e_shoff + (ehdr64.e_shentsize * (ehdr64.e_shnum - 1));
    file.seek(last_shdr_offset);
    ret = file.read(reinterpret_cast<char*>(&shdr64), sizeof(shdr64));
    if (ret < 0 || ret != sizeof(shdr64)) {
        qWarning() << "Read of ELF section header failed.";
        return -1;
    }

    last_section_end = bytesToHostByteOrder(reinterpret_cast<const uchar*>(&shdr64.sh_offset)) +
                       bytesToHostByteOrder(reinterpret_cast<const uchar*>(&shdr64.sh_size));
    return sht_end > last_section_end ? sht_end : last_section_end;
}

qint64 ElfBinary::readElf32(QFile &file)
{
    Elf32_Ehdr ehdr32;
    Elf32_Shdr shdr32;
    qint64 last_shdr_offset;
    qint64 ret;
    qint64 sht_end, last_section_end;

    file.seek(0); // Move to the beginning of the file
    ret = file.read(reinterpret_cast<char*>(&ehdr32), sizeof(ehdr32));
    if (ret < 0 || ret != sizeof(ehdr32)) {
        qWarning() << "Read of ELF header failed.";
        return -1;
    }

    if ((ehdr32.e_ident[EI_DATA] != ELFDATA2LSB) &&
        (ehdr32.e_ident[EI_DATA] != ELFDATA2MSB)) {
        qWarning() << "Unknown ELF data order" << ehdr32.e_ident[EI_DATA];
        return -1;
    }

    sht_end = ehdr32.e_shoff + (ehdr32.e_shentsize * ehdr32.e_shnum);
    last_shdr_offset = ehdr32.e_shoff + (ehdr32.e_shentsize * (ehdr32.e_shnum - 1));
    file.seek(last_shdr_offset);
    ret = file.read(reinterpret_cast<char*>(&shdr32), sizeof(shdr32));
    if (ret < 0 || ret != sizeof(shdr32)) {
        qWarning() << "Read of ELF section header failed.";
        return -1;
    }

    last_section_end = bytesToHostByteOrder(reinterpret_cast<const uchar*>(&shdr32.sh_offset)) +
                       bytesToHostByteOrder(reinterpret_cast<const uchar*>(&shdr32.sh_size));
    return sht_end > last_section_end ? sht_end : last_section_end;
}

qint64 ElfBinary::getElfSize(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open" << filePath;
        return -1;
    }

    uchar e_ident[EI_NIDENT];
    file.read(reinterpret_cast<char*>(e_ident), EI_NIDENT);

    qint64 size;
    if (e_ident[EI_CLASS] == ELFCLASS32) {
        size = readElf32(file);
    } else if (e_ident[EI_CLASS] == ELFCLASS64) {
        size = readElf64(file);
    } else {
        qWarning() << "Unknown ELF class" << e_ident[EI_CLASS];
        size = -1;
    }

    file.close();
    return size;
}