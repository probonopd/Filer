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

#include "customfileiconprovider.h"
#include "applicationbundle.h"

#include <QMimeDatabase>
#include <QDebug>

CustomFileIconProvider::CustomFileIconProvider()
{

}

/**
 * @brief CustomFileIconProvider::icon  Returns the icon for the given file info
 * @param info  The file info
 * @return  The icon
 */
QIcon CustomFileIconProvider::icon(const QFileInfo &info) const
{
    // Return the appropriate icon for the file based on its type

    // Check if the item is an application bundle and return the icon
    ApplicationBundle bundle(info.absoluteFilePath());
    if (bundle.isValid()) {
        return(QIcon(bundle.icon()));
    }


    // Get the MIME type of the file
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(info);
    qDebug() << "MIME type:" << mime.name();
    qDebug() << "MIME iconName:" << mime.iconName();
    QIcon icon = QIcon::fromTheme(mime.iconName());

    // Return the icon for the MIME type
    if(! icon.isNull()) {
        return icon;
    }


    // Use the base class method to return the default icon
    // in all other cases
    return QFileIconProvider::icon(info);
}