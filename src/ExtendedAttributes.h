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

/* We are using command line tools to modify extended attributes because
 * we can set those tools to setuid root. This allows us to set extended
 * attributes on files that we do not have write access to.
 */

#ifndef EXTENDEDATTRIBUTES_H
#define EXTENDEDATTRIBUTES_H

#include <QFile>
#include <QByteArray>

/**
 * @brief The ExtendedAttributes class provides functionality to read and write extended attributes of a file.
 */
class ExtendedAttributes
{
public:
    /**
     * @brief Constructs an ExtendedAttributes object for the specified file.
     * @param filePath The path of the file to work with.
     */
    ExtendedAttributes(const QString &filePath);

    /**
     * @brief Writes an extended attribute for the file.
     * @param attributeName The name of the attribute.
     * @param attributeValue The value of the attribute to be written.
     * @return True if the attribute was written successfully, false otherwise.
     */
    bool write(const QString &attributeName, const QByteArray &attributeValue);

    /**
     * @brief Reads the value of an extended attribute from the file.
     * @param attributeName The name of the attribute to read.
     * @return The value of the attribute, or an empty QByteArray if not found.
     */
    QByteArray read(const QString &attributeName);

private:
    QFile m_file; /**< The file associated with extended attributes. */
};

#endif // EXTENDEDATTRIBUTES_H
