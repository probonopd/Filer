#ifndef DBUSINTERFACE_H
#define DBUSINTERFACE_H

#include <QObject>

class DBusInterface : QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.FileManager1")

public:
    DBusInterface();
    Q_SCRIPTABLE void ShowFolders(const QStringList &uriList, const QString &startUpId);
    Q_SCRIPTABLE void ShowItems(const QStringList &uriList, const QString &startUpId);
    Q_SCRIPTABLE void ShowItemProperties(const QStringList &uriList, const QString &startUpId);

    Q_SCRIPTABLE void SortOrderForUrl(const QString &url, QString &role, QString &order);

private:
    bool m_isDaemon = false;
};

#endif // DBUSINTERFACE_H