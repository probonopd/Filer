#ifndef FILEOPERATIONMANAGER_H
#define FILEOPERATIONMANAGER_H

#include <QString>
#include <QStringList>

/**
 * @file FileOperationManager.h
 * @class FileOperationManager
 * @brief The FileOperationManager class provides functionality for copying and moving files with progress.
 */
class FileOperationManager {
public:
    /**
     * @brief Copies a list of files to a destination folder with progress.
     * @param fromPaths The list of source file paths.
     * @param toPath The destination folder path.
     */
    static void copyWithProgress(const QStringList& fromPaths, const QString& toPath);

    /**
     * @brief Moves a list of files to a destination folder with progress.
     * @param fromPaths The list of source file paths.
     * @param toPath The destination folder path.
     */
    static void moveWithProgress(const QStringList& fromPaths, const QString& toPath);

    /**
     * @brief Finds the path to the file operation binary, 'fileoperation'.
     * @note The binary should be shipped with this application. All file operation functionality is implemented in the binary.
     * @return The path to the binary.
     */
    static QString findFileOperationBinary();

private:
    /**
     * @brief Executes a file operation with progress.
     * @param fromPaths The list of source file paths.
     * @param toPath The destination folder path.
     * @param operation The operation to perform.
     */
    static void executeFileOperation(const QStringList& fromPaths, const QString& toPath, const QString& operation);

};

#endif // FILEOPERATIONMANAGER_H
