#ifndef COPYTHREAD_H
#define COPYTHREAD_H

#include <QThread>

class CopyThread : public QThread {
    Q_OBJECT

public:
    CopyThread(const QStringList& fromPaths, const QString& toPath, QObject* parent = nullptr);

    signals:
        void progress(int value);
        void copyFinished();
        void cancelCopyRequested();
        void error(const QString& errorMessage);

protected:
    void run() override;

private:
    qint64 calculateTotalSize();

    const QStringList& fromPaths;
    const QString& toPath;


};

#endif // COPYTHREAD_H