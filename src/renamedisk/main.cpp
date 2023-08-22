#include <QCoreApplication>
#include <QProcess>
#include <QTextStream>
#include <QDebug>
#include <QVector>
#include <QStandardPaths>
#include <QFileInfo>
#include <unistd.h>
#include <QDir>
#include <QThread>

void ensureAvailable(const QString &tool) {
    if (!QFileInfo(tool).isExecutable()) {
        qCritical() << "The command line tool" << tool << "is needed but is not available.";
        qCritical() << "Please install it, then try again. Exiting.";
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QTextStream out(stdout);
    QTextStream err(stderr);

    QStringList args = a.arguments();

    if (args.size() != 3) {
        qCritical() << "Usage: renamedisk /dev/diskXsY 'Disk Name";
        qCritical() << "       renamedisk /media/DISK1 'Disk Name";
        exit(1);
    }

    // Exit if we are not root
    if (geteuid() != 0) {
        qCritical() << "You must run this program as root. Exiting.";
        exit(1);
    }

    // Exit if we are not running on FreeBSD
    if (QStandardPaths::findExecutable("freebsd-version").isEmpty()) {
        qCritical() << "This program is only supported on FreeBSD. Exiting.";
        qCritical() << "TODO: Add support for other operating systems.";
        exit(1);
    }

    QString name = args.at(2);
    QString device;

    if (QFileInfo(args.at(1)).isDir()) {
        QProcess process;
        process.start("mount", QStringList() << "-p");
        process.waitForFinished();

        QByteArray output = process.readAllStandardOutput();
        QString outputStr(output);

        QStringList lines = outputStr.trimmed().split('\n');
        QVector<QString> mntdevs, mntpoints;

        for (const QString &line : lines) {
            // Split on one or multiple tabs
            QStringList parts = line.split(QRegExp("\\s+"));
            // qDebug() << parts;
            if (parts.size() >= 2) {
                mntdevs.append(parts[0]);
                mntpoints.append(parts[1]);
            }
        }

        if (mntpoints.contains(args.at(1))) {
            QString mntpoint = args.at(1);
            mntpoint = QFileInfo(mntpoint).canonicalFilePath();
            int index = mntpoints.indexOf(mntpoint);
            device = mntdevs[index];
        }
    } else {
        if (args.at(1).startsWith("/dev")) {
            device = args.at(1);
            device = QFileInfo(device).canonicalFilePath();
        } else {
            qCritical() << "Device must start with /dev. Exiting.";
            exit(1);
        }
    }

    if(device.isEmpty()) {
        qCritical() << "Device" << args.at(1) << "not found. Exiting.";
        exit(1);
    }

    if (!QFile::exists(device)) {
        qCritical() << "Device" << device << "does not exist";
        exit(1);
    }

    // If device is "/dev/fuse", we need to find out its device node in a different way
    // TODO: Find device node for /dev/fuse mounted devices
    if (device == "/dev/fuse") {
        qCritical() << "Device is /dev/fuse. Finding out its device node has not been implemented yet.";
        exit(1);
    }

    ensureAvailable("/usr/local/sbin/blkid");

    QProcess blkidProcess;
    blkidProcess.start("blkid", QStringList() << "-s" << "LABEL" << "-o" << "value" << device);
    blkidProcess.waitForFinished();
    QString oldName = blkidProcess.readAllStandardOutput().trimmed();

    qCritical() << "Old name:" << oldName;

    QProcess blkidFsProcess;
    blkidFsProcess.start("blkid", QStringList() << "-s" << "TYPE" << "-o" << "value" << device);
    blkidFsProcess.waitForFinished();
    QString filesystem = blkidFsProcess.readAllStandardOutput().trimmed();

    QString tool;
    QStringList toolArgs;

    if (filesystem.startsWith("ext")) {
        tool = "/usr/local/sbin/e2label";
        toolArgs << device << name;
    } else if (filesystem == "ntfs") {
        tool = "/usr/local/sbin/ntfslabel";
        toolArgs << device << name;
    } else if (filesystem.startsWith("reiser")) {
        tool = "/usr/local/sbin/reiserfstune";
        toolArgs << "-l" << name << device;
    } else if (filesystem == "vfat") {
        tool = "/usr/local/bin/mlabel";
        toolArgs << "-i" << device << "::" + name;
    } else if (filesystem == "exfat") {
        tool = "/usr/local/sbin/exfatlabel";
        toolArgs << device << name;
    } else if (filesystem == "ufs") {
        tool = "/sbin/tunefs";
        toolArgs << "-L" << name << device;
    } else {
        qCritical() << "The file system" << filesystem << "on" << device << "is not supported by this tool. Exiting.\n";
        exit(1);
    }

    ensureAvailable(tool);

    QProcess mountProcess;
    mountProcess.start("mount", QStringList() << "-p");
    mountProcess.waitForFinished();

    QByteArray mountOutput = mountProcess.readAllStandardOutput();
    QString mountOutputStr(mountOutput);

    QStringList mntdevs, mntpoints, mntfstypes, mntopts;

    QStringList mountLines = mountOutputStr.trimmed().split('\n');
    for (const QString &line : mountLines) {
        QStringList parts = line.split(QRegExp("\\s+"));
        if (parts.size() >= 4) {
            mntdevs.append(parts[0]);
            mntpoints.append(parts[1]);
            mntfstypes.append(parts[2]);
            mntopts.append(parts[3]);
        }
    }

    bool wasMounted = false;
    QString mountMountpoint;
    QString mountFs;
    QString mountOptions;

    if (mntdevs.contains(device)) {
        int index = mntdevs.indexOf(device);
        wasMounted = true;
        mountMountpoint = mntpoints.at(index);
        mountFs = mntfstypes.at(index);
        mountOptions = mntopts.at(index);
    }

    if (wasMounted) {
        qDebug() << "Device" << device << "was mounted on" << mountMountpoint << "with file system" << mountFs << "and options" << mountOptions << ". Unmounting...";
        QProcess umountProcess;
        umountProcess.start("umount", QStringList() << device);
        umountProcess.waitForFinished();
        if (umountProcess.exitCode() != 0) {
            qCritical() << "Could not unmount" << device << ". Exiting.";
            exit(1);
        }
        // Remove old mount point
        QDir mountDir(mountMountpoint);
        if (mountDir.exists()) {
            if (!mountDir.rmdir(mountMountpoint)) {
                qCritical() << "Could not remove old mount point" << mountMountpoint << ". Exiting.";
                exit(1);
            }
        }
    } else {
        qDebug() << "Device" << device << "was not mounted, so it will not be unmounted nor remounted.";
    }

    qDebug() << "Renaming device" << device << "to" << name << "...";

    toolArgs.prepend(tool);

    QProcess renameProcess;
    renameProcess.start(toolArgs.first(), toolArgs.mid(1));
    qDebug() << renameProcess.program() + " '" + renameProcess.arguments().join("' '") + "'";
    renameProcess.waitForFinished();

    if (renameProcess.exitCode() != 0) {
        qCritical() << "Could not rename" << device << ". Exiting.";
        exit(1);
    }

    // Sync
    QProcess syncProcess;
    syncProcess.start("sync");
    syncProcess.waitForFinished();

    QProcess newLabelProcess;
    newLabelProcess.start("blkid", QStringList() << "-s" << "LABEL" << "-o" << "value" << device);
    newLabelProcess.waitForFinished();
    QString newName = newLabelProcess.readAllStandardOutput().trimmed();

    // Sometimes the new name is not immediately available
    // because the device is still busy. We should wait until it is
    // available. Retry a few times, and if it still fails, exit with
    // an error.
    for (int i = 0; i < 10; i++) {
        if (newName != oldName) {
            break;
        }
        qDebug() << "New name is still" << newName << ". Waiting...";
        QThread::sleep(1);
        newLabelProcess.start("blkid", QStringList() << "-s" << "LABEL" << "-o" << "value" << device);
        newLabelProcess.waitForFinished();
        newName = newLabelProcess.readAllStandardOutput().trimmed();
    }

    qDebug() << "New name:" << newName;

    if (wasMounted) {
        QString newMountpoint = mountMountpoint.left(mountMountpoint.lastIndexOf('/')) + '/' + newName;
        qDebug() << "Creating mount point" << newMountpoint << "...";
        QDir mountpointDir(newMountpoint);
        bool success = mountpointDir.mkpath(".");
        if (!success) {
            qCritical() << "Could not create mount point" << newMountpoint << ". Exiting.";
            exit(1);
        }
        qDebug() << "Remounting device" << device << "on" << newMountpoint << "...";
        QProcess *mountAgainProcess = new QProcess;
        mountAgainProcess->setProgram("mount");
        QStringList mountArgs = QStringList() << "-t" << mountFs << "-o" << mountOptions << device << newMountpoint;
        mountAgainProcess->setArguments(mountArgs);
        qDebug() << mountAgainProcess->program() + " '" + mountAgainProcess->arguments().join("' '") + "'";
        mountAgainProcess->start();
        mountAgainProcess->waitForFinished();
        if (mountAgainProcess->exitCode() != 0) {
            qCritical() << "Could not mount" << device << ". Exiting.";
            exit(1);
        }
    }

    return 0;
}
