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
#ifndef FILER_LAUNCHDB_H
#define FILER_LAUNCHDB_H

#include <QString>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>

/**
 * @brief The LaunchDB class provides functionality to retrieve the default application associated with a file.
 *
 * The class represents the launch database that holds information about known applications,
 * which MIME types they support, and which default application should be used for which MIME type.
 * The launch database is implemented using directories and symlinks (e.g. ~/.local/share/launch/MIME/).
 * It is populated and managed by the 'launch' and `open` command line tools from
 * https://github.com/helloSystem/launch/.
 */
class LaunchDB {
public:
    LaunchDB();

    ~LaunchDB();

    /**
     * @brief Retrieves the default application associated with the specified file.
     * @param fileInfo The QFileInfo object representing the file.
     * @return The path to the default application for the file, or an empty QString if not found.
     */
    QString applicationForFile(const QFileInfo &fileInfo) const;

private:
    QMimeDatabase *db; // Member variable to store the QMimeDatabase object
};

#endif // FILER_LAUNCHDB_H
