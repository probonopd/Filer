#ifndef COPYMANAGER_H
#define COPYMANAGER_H

#include <QObject>
#include <QStringList>

class CopyProgressDialog;
class CopyThread;

class CopyManager : public QObject {
    Q_OBJECT

public:
    explicit CopyManager(QObject* parent = nullptr);
    ~CopyManager();

    void copyWithProgress(const QStringList& fromPaths, const QString& toPath);

    signals:
        void copyFinished();
        void copyCanceled();
        void errorOccured(const QString& errorMessage);

private slots:
    void onCopyProgress(int progress);
    void onCopyFinished();
    void onCancelCopy();
    void onErrorOccurred(const QString& errorMessage);

private:
    CopyProgressDialog* progressDialog;
    CopyThread* copyThread;
};

#endif // COPYMANAGER_H