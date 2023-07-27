// customfilesystemmodel.cpp
#include "CustomFileSystemModel.h"
#include "ExtendedAttributes.h"
#include <QDebug>
#include "ApplicationBundle.h"


CustomFileSystemModel::CustomFileSystemModel(QObject* parent)
        : QFileSystemModel(parent)
{
    LaunchDB ldb;
}

// Implementation of the additional method:
QByteArray CustomFileSystemModel::readExtendedAttribute(const QModelIndex& index, const QString& attributeName) const
{
    if (!index.isValid() || index.column() != 0) {
        return QByteArray(); // Invalid index or not pointing to a file entry.
    }

    QString filePath = QFileSystemModel::filePath(index);
    QByteArray attributeValue;

    ExtendedAttributes ea(filePath);
    attributeValue = ea.read(attributeName);

    return attributeValue;
}

// Returns the open-with extended attribute for the given file, or the default application for the file type if the
// attribute is not set (based on information from the LaunchDB).
QString CustomFileSystemModel::openWith(const QFileInfo& fileInfo) const {
    // NOTE: Would like to do this with an index, but don't have a valid index when this gets called for unknown reasons.
    // So we'll use the QFileInfo instead for now.
    QString filePath = fileInfo.absoluteFilePath();

    // If we already have the attribute, return it
    // Try to find this file in the model
    QModelIndex index = CustomFileSystemModel::index(filePath);
    if (index.isValid() && openWithAttributes.contains(index)) {
        return openWithAttributes[index];
    }

    // Otherwise, get it from the file's extended attributes
    QString attributeValue;
    ExtendedAttributes ea(filePath);
    attributeValue = QString(ea.read("open-with"));

    // If it's empty, get it from the LaunchDB
    if (attributeValue.isEmpty()) {
        // Get it from the LaunchDB
        LaunchDB ldb;
        attributeValue = QString(ldb.applicationForFile(filePath));

    }

    if (index.isValid()) {
        // If we found it, store it in the model for future use
        qDebug() << "Updating model with open-with attribute for " << filePath << ": " << attributeValue;
        openWithAttributes[index] = attributeValue.toUtf8();
    } else {
        qDebug() << "Did not find " << filePath << " in the model. FIXME: Find the reason why this happens. Working around for now.";
        // Once we have solved this condition, we can use an index instead of a QFileInfo for this method.
    }

    return attributeValue;
}
