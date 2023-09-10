#ifndef ICONPROVIDER_H
#define ICONPROVIDER_H

#include <QFileIconProvider>
#include "CombinedIconCreator.h"
#include <QAbstractItemModel>

class CustomFileIconProvider : public QFileIconProvider
{
public:
    CustomFileIconProvider();
    ~CustomFileIconProvider();

    // Override the icon() function to provide custom icons.
    QIcon icon(const QFileInfo &info) const override;
    QIcon documentIcon(const QFileInfo &info, QString openWith) const;

    void setModel(QAbstractItemModel *model);

private:
    CombinedIconCreator* m_iconCreator;

    QIcon userIcon(const QFileInfo &info) const;
};

#endif // ICONPROVIDER_H
