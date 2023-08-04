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

#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <QDialog>
#include <QFileInfo>

namespace Ui {
    class InfoDialog;
}

/**
 * @brief The InfoDialog class represents a dialog to display information about a file or directory.
 */
class InfoDialog : public QDialog
{
Q_OBJECT

public:
    /**
     * @brief Constructs an InfoDialog with the given file path.
     * @param filePath The path of the file or directory to display information for.
     * @param parent The parent widget (optional).
     */
    explicit InfoDialog(const QString &filePath, QWidget *parent = nullptr);

    /**
     * @brief Destructor.
     */
    ~InfoDialog();

private:
    Ui::InfoDialog *ui; /**< The user interface components. */
    QString filePath; /**< The path of the file or directory. */
    QFileInfo fileInfo; /**< File information. */

    /**
     * @brief Converts file permissions to a human-readable string.
     * @param permissions The file permissions to convert.
     * @return A string representation of the file permissions.
     */
    QString getPermissionsString(QFile::Permissions permissions);

    /**
     * @brief Sets up the information to be displayed in the dialog.
     */
    void setupInformation();

private slots:
    /**
     * @brief Slot to open the file using the default associated application.
     */
    void openFile();
};

#endif // INFODIALOG_H
