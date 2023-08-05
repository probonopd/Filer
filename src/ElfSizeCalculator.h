#ifndef ELFSIZECALCULATOR_H
#define ELFSIZECALCULATOR_H

#include <QString>
#include <elf.h>

/**
 * @file ElfSizeCalculator.h
 * @brief The ElfSizeCalculator class provides static methods for calculating ELF file sizes.
 *
 * This class provides static methods to calculate the size of ELF (Executable and Linkable Format) files.
 * It supports both 32-bit and 64-bit ELF formats. The main method to use is CalculateElfSize, which takes
 * a file path as input and returns the calculated ELF size in bytes.
 */
class ElfSizeCalculator
{
public:
    /**
     * @brief Calculate the ELF size for the given file path.
     * @param filePath The path to the ELF file.
     * @return The calculated ELF size in bytes.
     */
    static qint64 calculateElfSize(const QString& filePath);

private:
    /**
     * @brief Internal helper function to calculate ELF size.
     * @param filePath The path to the ELF file.
     * @return The calculated ELF size in bytes.
     * @details This function reads the ELF header, checks its validity, and calculates the size based on
     * the architecture (32-bit or 64-bit).
     */
    static qint64 CalculateElfSizeInternal(const QString& filePath);

    /**
     * @brief Helper function to check if ELF header magic is valid.
     * @param header The ELF header.
     * @return True if the magic is valid, false otherwise.
     */
    static bool IsElfHeaderMagicValid(const Elf64_Ehdr& header);

    /**
     * @brief Helper function to calculate ELF size for 64-bit architecture.
     * @param header The ELF header.
     * @param fileSize The size of the ELF file.
     * @return The calculated ELF size in bytes.
     */
    static qint64 CalculateElfSize64(const Elf64_Ehdr& header, qint64 fileSize);

    /**
     * @brief Helper function to calculate ELF size for 32-bit architecture.
     * @param header The ELF header.
     * @param fileSize The size of the ELF file.
     * @return The calculated ELF size in bytes.
     */
    static qint64 CalculateElfSize32(const Elf64_Ehdr& header, qint64 fileSize);

    /**
     * @brief Helper function to print error messages.
     * @param context The context or location of the error.
     * @param error The error message.
     */
    static void PrintError(const QString& context, const QString& error);
};

#endif // ELFSIZECALCULATOR_H
