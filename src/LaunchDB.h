#ifndef FILER_LAUNCHDB_H
#define FILER_LAUNCHDB_H

#include <QString>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>

class LaunchDB {
public:
    LaunchDB(); // Constructor
    ~LaunchDB(); // Destructor

    QString applicationForFile(const QFileInfo &fileInfo) const;

private:
    QMimeDatabase *db; // Member variable to store the QMimeDatabase object
};

#endif // FILER_LAUNCHDB_H
