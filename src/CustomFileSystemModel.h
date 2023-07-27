// CustomFileSystemModel.h
#pragma once

#include <QFileSystemModel>
#include <QByteArray>
#include "LaunchDB.h"
#include "CombinedIconCreator.h"

class CustomFileSystemModel : public QFileSystemModel
{
Q_OBJECT
public:
    explicit CustomFileSystemModel(QObject* parent = nullptr);

    // Additional method (updated with const qualifier):
    QByteArray readExtendedAttribute(const QModelIndex& index, const QString& attributeName) const;

    // Public method to safely access data from CustomFileSystemModel.
    QVariant fileData(const QModelIndex& index, int role) const;

    // Public method to access the "open-with" attribute.
    // NOTE: Would like to do this with an index, but don't have a valid index when this gets called for unknown reasons.
    // So we'll use the QFileInfo instead for now.
    QString openWith(const QFileInfo& fileInfo) const;

private:
    // Private member variable to store "open-with" attributes.
    mutable QMap<QModelIndex, QByteArray> openWithAttributes;

    LaunchDB ldb;

};
