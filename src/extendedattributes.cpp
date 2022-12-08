/*-
 * Copyright (c) 2022 Simon Peter <probono@puredarwin.org>
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

#include "extendedattributes.h"

#include <QProcess>
#include <QStringList>
#include <QTextStream>
#include <QDebug>

ExtendedAttributes::ExtendedAttributes(const QString &filePath)
        : m_file(filePath)
{
}

bool ExtendedAttributes::write(const QString &attributeName, const QByteArray &attributeValue)
{
    qDebug() << "Trying to write extended attribute" << attributeName << "with value" << attributeValue;

    if (!m_file.exists()) {
        // Error: File does not exist
        qWarning() << "ExtendedAttributes::write(): File does not exist";
        return false;
    }

#if defined(__unix__) || defined(__APPLE__)
    // Write the extended attribute to the file in the "user" namespace
    qDebug() << "Writing extended attribute" << attributeName << "with value" << attributeValue << "to file" << m_file.fileName();
    QProcess extattr;
    extattr.start("setextattr", QStringList() << "-hq" << "user" << attributeName << attributeValue << m_file.fileName());
    if (!extattr.waitForFinished()) {
        // Error writing extended attribute to user namespace
        qWarning() << "ExtendedAttributes::write(): Error writing extended attribute to user namespace";
        return false;
    } else {
        // Extended attribute was written successfully
        qDebug() << "Extended attribute was written successfully";
    }
#elif defined(__linux__)
    qDebug() << "Writing extended attribute" << attributeName << "with value" << attributeValue << "to file" << m_file.fileName();
    // Write the extended attribute to the file in the "user" namespace
    QProcess xattr;
    xattr.start("setxattr", QStringList() << "-hq" << "user" << attributeName << attributeValue << m_file.fileName());
    if (!xattr.waitForFinished()) {
        // Error writing extended attribute to user namespace
        qWarning() << "ExtendedAttributes::write(): Error writing extended attribute to user namespace";
        return false;
    } else {
        // Extended attribute was written successfully
        qDebug() << "Extended attribute was written successfully";
    }
#endif
    return true;
    qDebug() << "ExtendedAttributes::wrote(): " << attributeName << " " << attributeValue;
}

QByteArray ExtendedAttributes::read(const QString &attributeName)
{
    qDebug() << "Trying to read extended attribute" << attributeName;

    if (!m_file.exists()) {
        // Error: File does not exist
        qWarning() << "ExtendedAttributes::read(): File does not exist";
        return QByteArray();
    }

#if defined(__unix__) || defined(__APPLE__)
    // Read the extended attribute from the file in the "user" namespace
    qDebug() << "Reading extended attribute" << attributeName << "from file" << m_file.fileName();
    QProcess extattr;
    extattr.start("getextattr", QStringList() << "-hq" << "user" << attributeName << m_file.fileName());
    if (!extattr.waitForFinished()) {
        // Error reading extended attribute from user namespace
        qWarning() << "ExtendedAttributes::read(): Error reading extended attribute from user namespace";
        return QByteArray();
    } else {
        // Extended attribute was read successfully
        qDebug() << "Extended attribute was read successfully";
    }
    QByteArray attributeValue = extattr.readAllStandardOutput().trimmed();
    qDebug() << "ExtendedAttributes::read():" << attributeName << " " << attributeValue;
    return attributeValue;
#elif defined(__linux__)
    // Read the extended attribute from the file in the "user" namespace
    qDebug() << "Reading extended attribute" << attributeName << "from file" << m_file.fileName();
    QProcess xattr;
    xattr.start("getxattr", QStringList() << "-hq" << "user" << attributeName << m_file.fileName());
    if (!xattr.waitForFinished()) {
        // Error reading extended attribute from user namespace
        qWarning() << "ExtendedAttributes::read(): Error reading extended attribute from user namespace";
        return QByteArray();
    } else {
        // Extended attribute was read successfully
        qDebug() << "Extended attribute was read successfully";
    }

    QByteArray attributeValue = xattr.readAllStandardOutput().trimmed();
    qDebug() << "ExtendedAttributes::read():" << attributeName << " " << attributeValue;
    return attributeValue;
#endif
}