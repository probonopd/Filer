#ifndef TRASHHANDLER_H
#define TRASHHANDLER_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QMessageBox>

class TrashHandler : public QObject {
Q_OBJECT
public:
    TrashHandler(QWidget *parent = nullptr);

    // Move a file/directory to the "Trash" (Virtual Trash)
    void moveToTrash(const QStringList& paths);

    // Empty the "Trash" (Delete all files and directories in the Virtual Trash)
    bool emptyTrash();

    // Get the path to the "Trash" directory
    QString getTrashPath() const;

private:
    QString m_trashPath;
    QWidget *m_parent;
    bool m_dialogShown = false;
};

#endif // TRASHHANDLER_H
