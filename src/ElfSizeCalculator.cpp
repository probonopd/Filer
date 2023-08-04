#include "ElfSizeCalculator.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>

qint64 ElfSizeCalculator::CalculateElfSize(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        PrintError("ioReader", file.errorString());
        return 0;
    }

    const qint64 fileSize = file.size();
    if (fileSize < sizeof(Elf64_Ehdr))
    {
        PrintError("ioReader", "Invalid ELF file");
        return 0;
    }

    Elf64_Ehdr header;
    if (file.read(reinterpret_cast<char*>(&header), sizeof(header)) != sizeof(header))
    {
        PrintError("elfsize read identifier", file.errorString());
        return 0;
    }

    if (!IsElfHeaderMagicValid(header))
    {
        PrintError("elfsize read identifier", "Bad magic number");
        return 0;
    }

    // Process by architecture
    if (header.e_ident[EI_CLASS] == ELFCLASS64)
    {
        return CalculateElfSize64(header, fileSize);
    }
    else if (header.e_ident[EI_CLASS] == ELFCLASS32)
    {
        return CalculateElfSize32(header, fileSize);
    }
    else
    {
        PrintError("elfsize", "Unsupported elf architecture");
        return 0;
    }
}

bool ElfSizeCalculator::IsElfHeaderMagicValid(const Elf64_Ehdr& header)
{
    return (header.e_ident[EI_MAG0] == ELFMAG0 &&
            header.e_ident[EI_MAG1] == ELFMAG1 &&
            header.e_ident[EI_MAG2] == ELFMAG2 &&
            header.e_ident[EI_MAG3] == ELFMAG3);
}

qint64 ElfSizeCalculator::CalculateElfSize64(const Elf64_Ehdr& header, qint64 fileSize)
{
    const qint64 shoff = header.e_shoff;
    const qint64 shnum = header.e_shnum;
    const qint64 shentsize = header.e_shentsize;

    return shoff + (shentsize * shnum);
}

qint64 ElfSizeCalculator::CalculateElfSize32(const Elf64_Ehdr& header, qint64 fileSize)
{
    const qint64 shoff = header.e_shoff;
    const qint64 shnum = header.e_shnum;
    const qint64 shentsize = header.e_shentsize;

    return shoff + (shentsize * shnum);
}

void ElfSizeCalculator::PrintError(const QString& context, const QString& error)
{
    qCritical() << "ERROR" << context << ":" << error;
}
