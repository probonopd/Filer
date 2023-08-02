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

#ifndef ELFBINARY_H
#define ELFBINARY_H

#include <QObject>
#include <QFile>

/**
 * @brief The ElfBinary class provides functionality to read information from ELF binary files.
 *
 * The class allows reading the size of ELF files, supporting both 32-bit and 64-bit ELF formats.
 */
class ElfBinary : public QObject
{
Q_OBJECT

public:
    /**
     * @brief Constructs an ElfBinary object.
     * @param parent The parent QObject (default is nullptr).
     */
    explicit ElfBinary(QObject *parent = nullptr);

    /**
     * @brief Gets the size of the ELF file specified by the file path.
     * @param filePath The path to the ELF file.
     * @return The size of the ELF file in bytes, or -1 if an error occurred.
     */
    Q_INVOKABLE qint64 getElfSize(const QString &filePath);

private:
    qint64 readElf64(QFile &file);
    qint64 readElf32(QFile &file);
    quint64 bytesToHostByteOrder(const uchar *bytes, int size = sizeof(quint64));

    /**
     * @brief Structure representing the ELF header for 64-bit format.
     */
    typedef struct {
        uchar e_ident[16];
        quint16 e_type;
        quint16 e_machine;
        quint32 e_version;
        quint64 e_entry;
        quint64 e_phoff;
        quint64 e_shoff;
        quint32 e_flags;
        quint16 e_ehsize;
        quint16 e_phentsize;
        quint16 e_phnum;
        quint16 e_shentsize;
        quint16 e_shnum;
        quint16 e_shstrndx;
    } Elf64_Ehdr;

    /**
     * @brief Structure representing the ELF header for 32-bit format.
     */
    typedef struct {
        uchar e_ident[16];
        quint16 e_type;
        quint16 e_machine;
        quint32 e_version;
        quint32 e_entry;
        quint32 e_phoff;
        quint32 e_shoff;
        quint32 e_flags;
        quint16 e_ehsize;
        quint16 e_phentsize;
        quint16 e_phnum;
        quint16 e_shentsize;
        quint16 e_shnum;
        quint16 e_shstrndx;
    } Elf32_Ehdr;

    /**
     * @brief Structure representing the ELF section header for 64-bit format.
     */
    typedef struct elf64_shdr {
        quint32 sh_name;
        quint32 sh_type;
        quint64 sh_flags;
        quint64 sh_addr;
        quint64 sh_offset;
        quint64 sh_size;
        quint32 sh_link;
        quint32 sh_info;
        quint64 sh_addralign;
        quint64 sh_entsize;
    } Elf64_Shdr;

    /**
     * @brief Structure representing the ELF section header for 32-bit format.
     */
    typedef struct elf32_shdr {
        quint32 sh_name;
        quint32 sh_type;
        quint32 sh_flags;
        quint32 sh_addr;
        quint32 sh_offset;
        quint32 sh_size;
        quint32 sh_link;
        quint32 sh_info;
        quint32 sh_addralign;
        quint32 sh_entsize;
    } Elf32_Shdr;

};
#endif // ELFBINARY_H
