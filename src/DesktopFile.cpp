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

#include "DesktopFile.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QTextStream>
#include <QDebug>
#include <QIcon>

DesktopFile::DesktopFile() { }

QString DesktopFile::getValue(const QString &filename, const QString &key)
{
    QString value;
    QFile file(filename);
    // qDebug() << "Searching for" << key << "in" << filename;
    // Search for a line starting with "Icon=" in the file
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith(key)) {
                // Split the line at "=" and take the second part, trim it
                value = line.split("=").at(1).trimmed();
                break;
            }
        }
        file.close();
    }

    // Print error message if icon is not found
    if (value.isEmpty()) {
        qDebug() << key << "not found in" << filename;
    }

    return value;
}

QString DesktopFile::getIcon(const QString &filename)
{
    return getValue(filename, "Icon");
}

QString DesktopFile::getName(const QString &filename)
{
    return getValue(filename, "Name");
}

bool DesktopFile::isCommandLineTool(const QString &filename)
{
    qDebug() << "Checking if" << filename << "is a command line tool";
    if (getValue(filename, "Terminal") == "true") {
        qDebug() << filename << "is a command line tool";
        return true;
    }
    return false;
}