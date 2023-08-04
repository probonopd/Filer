/**
 * @file elf_size_calculator.h
 * @brief Calculate the size of an ELF file based on the information in the ELF header.
 *
 * The ElfSizeCalculator class provides methods to calculate
 * the size of an ELF (Executable and Linkable Format) file in bytes based on the information
 * available in the ELF header. The class supports both 32-bit and 64-bit ELF files.
 * FIXME: On 64-bit machines, the size of 32-bit ELF files is calculated incorrectly.
 * FIXME: On 32-bit machines, the size of 64-bit ELF files is possibly also calculated incorrectly.
 */

#ifndef ELF_SIZE_CALCULATOR_H
#define ELF_SIZE_CALCULATOR_H

#include <elf.h>
#include <QString>

/**
 * @class ElfSizeCalculator
 * @brief Calculate the size of an ELF file based on the information in the ELF header.
 *
 * The ElfSizeCalculator class provides methods to calculate the size of an ELF file in bytes
 * based on the information available in the ELF header.
 */
class ElfSizeCalculator
{
public:
    /**
     * @brief Calculate the size of the ELF file.
     *
     * This function takes the path of an ELF file as input and returns the size of the ELF
     * file in bytes. It reads the ELF header to determine the size.
     *
     * @param filePath The path of the ELF file.
     * @return The size of the ELF file in bytes, or 0 if an error occurs.
     */
    qint64 CalculateElfSize(const QString& filePath);

private:
    /**
     * @brief Check if the ELF header magic number is valid.
     *
     * This function checks the magic number in the ELF header to verify if the file is a
     * valid ELF file.
     *
     * @param header The ELF header structure.
     * @return True if the magic number is valid, False otherwise.
     */
    bool IsElfHeaderMagicValid(const Elf64_Ehdr& header);

    /**
     * @brief Calculate the ELF size for 64-bit architecture.
     *
     * This function calculates the size of an ELF file for 64-bit architecture based on the
     * information in the ELF header.
     *
     * @param header The ELF header structure.
     * @param fileSize The total size of the ELF file in bytes.
     * @return The size of the ELF file in bytes.
     */
    qint64 CalculateElfSize64(const Elf64_Ehdr& header, qint64 fileSize);

    /**
     * @brief Calculate the ELF size for 32-bit architecture.
     *
     * This function calculates the size of an ELF file for 32-bit architecture based on the
     * information in the ELF header.
     *
     * @param header The ELF header structure.
     * @param fileSize The total size of the ELF file in bytes.
     * @return The size of the ELF file in bytes.
     */
    qint64 CalculateElfSize32(const Elf64_Ehdr& header, qint64 fileSize);

    /**
     * @brief Print error messages to the console.
     *
     * This function is used to print error messages to the console when an error occurs
     * during ELF size calculation.
     *
     * @param context A string that explains the context of the error.
     * @param error The error message to be printed.
     */
    void PrintError(const QString& context, const QString& error);
};

#endif // ELF_SIZE_CALCULATOR_H
