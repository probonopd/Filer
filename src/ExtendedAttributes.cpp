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

#include "ExtendedAttributes.h"

#include <QProcess>
#include <QStringList>
#include <QTextStream>
#include <QDebug>

#if defined(__unix__) || defined(__APPLE__)
#include <sys/types.h>
#include <sys/extattr.h>
#elif defined(__linux__)
#include <sys/types.h>
#include <sys/xattr.h>
#endif

ExtendedAttributes::ExtendedAttributes(const QString &filePath) : m_file(filePath) { }

bool ExtendedAttributes::write(const QString &attributeName, const QByteArray &attributeValue)
{
    // qDebug() << "Trying to write extended attribute" << attributeName << "with value" << attributeValue;

    if (!m_file.exists()) {
        // Error: File does not exist
        qWarning() << "ExtendedAttributes::write(): File does not exist";
        return false;
    }

#if defined(__unix__) || defined(__APPLE__)

    // NOTE: This is faster than using QProcess, but it means that we cannot
    // write extended attributes to files that we have no write access to

    int result = extattr_set_file(m_file.fileName().toUtf8().constData(),
                                  EXTATTR_NAMESPACE_USER,
                                  attributeName.toUtf8().constData(),
                                  attributeValue.constData(),
                                  attributeValue.length());
    if (result == -1) {
        // Error writing extended attribute to user namespace
        qWarning() << "Error writing extended attribute" << attributeName << "with value" << attributeValue
                 << "to file" << m_file.fileName();
        return false;
    } else {
        // Extended attribute was written successfully
        qDebug() << "Written extended attribute" << attributeName << "with value" << attributeValue
                 << "to file" << m_file.fileName();
    }

#elif defined(__linux__)

    qDebug() << "Writing extended attribute" << attributeName << "with value" << attributeValue
             << "to file" << m_file.fileName();
    // Write the extended attribute to the file in the "user" namespace
    QProcess xattr;
    xattr.start("setfattr",
                QStringList() << "-n" << "user." + attributeName
                            << "-v" << attributeValue
                            << m_file.fileName());
    if (!xattr.waitForFinished()) {
        // Error writing extended attribute to user namespace
        qWarning() << "ExtendedAttributes::write(): Error writing extended attribute to user "
                      "namespace";
        return false;
    } else {
        // Extended attribute was written successfully
        // qDebug() << "Extended attribute was written successfully";
    }

#endif

    return true;
    qDebug() << "ExtendedAttributes::wrote(): " << attributeName << " " << attributeValue;
}

QByteArray ExtendedAttributes::read(const QString &attributeName) {
    // qDebug() << "Trying to read extended attribute" << attributeName;

    if (!m_file.exists()) {
        // Error: File does not exist
        qWarning() << "ExtendedAttributes::read(): File does not exist";
        return QByteArray();
    }

#if defined(__unix__) || defined(__APPLE__)

    // NOTE: This implementation is faster than using QProcess, but it means that we cannot get
    // extended attributes from files that we do not have read access to.

    // Determine the size of the extended attribute data
    ssize_t dataSize = extattr_get_file(
            m_file.fileName().toUtf8().constData(),
            EXTATTR_NAMESPACE_USER,
            attributeName.toUtf8().constData(),
            NULL, 0
    );

    if (dataSize < 0) {
        return QByteArray();
    }

    // Allocate a buffer to store the extended attribute data
    void* dataBuffer = malloc(dataSize);
    if (dataBuffer == NULL) {
        return QByteArray();
    }

    // Retrieve the extended attribute data
    ssize_t actualDataSize = extattr_get_file(
            m_file.fileName().toUtf8().constData(),
            EXTATTR_NAMESPACE_USER,
            attributeName.toUtf8().constData(),
            dataBuffer, dataSize
    );

    if (actualDataSize < 0) {
        free(dataBuffer);
        return QByteArray();
    }

    QByteArray attributeValue = QByteArray::fromRawData((char*)dataBuffer, actualDataSize);
    free(dataBuffer);
    return attributeValue;

#elif defined(__linux__)

    // Read the extended attribute from the file in the "user" namespace
    qDebug() << "Reading extended attribute" << attributeName << "from file" << m_file.fileName();
    QProcess xattr;
    xattr.start("getfattr",
                QStringList() << "-n" << "user." + attributeName
                            << "-d" << m_file.fileName());
    if (!xattr.waitForFinished()) {
        // Error reading extended attribute from user namespace
        qWarning() << "ExtendedAttributes::read(): Error reading extended attribute from user "
                      "namespace";
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

bool ExtendedAttributes::clear(const QString &attributeName) {
    // qDebug() << "Trying to delete extended attribute" << attributeName;

    if (!m_file.exists()) {
        // Error: File does not exist
        qWarning() << "ExtendedAttributes::delete(): File does not exist";
        return false;
    }

#if defined(__unix__) || defined(__APPLE__)
    // Delete the extended attribute from the file in the "user" namespace
    // qDebug() << "Deleting extended attribute" << attributeName << "from file" << m_file.fileName();
    QProcess extattr;
    extattr.start("rmextattr",
                  QStringList() << "-q"
                                << "user" << attributeName << m_file.fileName());
    if (!extattr.waitForFinished()) {
        // Error deleting extended attribute from user namespace
        qWarning() << "ExtendedAttributes::delete(): Error deleting extended attribute from user "
                      "namespace";
        return false;
    } else {
        // Extended attribute was deleted successfully
        // qDebug() << "Extended attribute was deleted successfully";
    }

#elif defined(__linux__)

    // Delete the extended attribute from the file in the "user" namespace
    qDebug() << "Deleting extended attribute" << attributeName << "from file" << m_file.fileName();
    QProcess xattr;
    xattr.start("setfattr",
                QStringList() << "-x" << "user." + attributeName
                            << m_file.fileName());
    if (!xattr.waitForFinished()) {
        // Error deleting extended attribute from user namespace
        qWarning() << "ExtendedAttributes::delete(): Error deleting extended attribute from user "
                      "namespace";
        return false;
    } else {
        // Extended attribute was deleted successfully
        qDebug() << "Extended attribute was deleted successfully";
    }

#endif

    return true;
}