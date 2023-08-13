#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QStyle>
#include <QDesktopWidget>
#include <QTranslator>
#include "MainWindow.h"
#include <QMessageBox>
#include <QFile>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("fileoperation");

    QDesktopWidget desktop;

    QCommandLineParser parser;
    parser.setApplicationDescription("A command line tool for copying and moving files with a graphical progress dialog.");
    parser.addHelpOption();
    parser.addVersionOption();

    // Define custom options
    QCommandLineOption copyOption("copy", "Copy files.");
    QCommandLineOption moveOption("move", "Move files.");
    parser.addOption(copyOption);
    parser.addOption(moveOption);

    // Process the command line arguments
    parser.process(a); // Use 'a' instead of 'app'

    if (parser.isSet("copy")) {
        qDebug() << "Copying files...";
        QStringList args = parser.positionalArguments();
        if (args.size() < 2) {
            qWarning() << "Usage: --copy <source path> [<source path> ...] <target path>";
            return 1;
        }
        QString targetPath = args.last();
        args.removeLast(); // Remove the target path from the list
        qDebug() << "Source paths:" << args;
        qDebug() << "Target path:" << targetPath;

        // Perform the copy operation
        MainWindow w;
        w.startCopyWithProgress(args, targetPath);

        // Center the main window on the screen
        w.setGeometry(
                QStyle::alignedRect(
                        Qt::LeftToRight,
                        Qt::AlignCenter,
                        w.size(),
                        desktop.availableGeometry() // Get screen geometry
                )
        );

        return a.exec();
    }
    else if (parser.isSet("move")) {
        qDebug() << "Moving files...";
        QStringList args = parser.positionalArguments();
        if (args.size() < 2) {
            qWarning() << "Usage: --move <source path> [<source path> ...] <target path>";
            return 1;
        }
        QString targetPath = args.last();
        args.removeLast(); // Remove the target path from the list
        qDebug() << "Source paths:" << args;
        qDebug() << "Target path:" << targetPath;

        // Perform the move operation
        MainWindow w;
        w.startCopyWithProgress(args, targetPath);
        // Get the result of the operation
        int result = a.exec();
        qDebug() << "Result:" << result;
        // If the result is 0, then the operation was successful
        if (result == 0) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                    nullptr,
                    "Delete source files?",
                    "Do you want to delete the following?\n\n" + args.join("\n"),
                    QMessageBox::Yes | QMessageBox::No
            );
            if (reply == QMessageBox::Yes) {
                qDebug() << "Deleting source files...";
                // Recursively delete the source files
                for (const QString &path : args) {
                    QDir dir(path);
                    if (dir.exists()) {
                        dir.removeRecursively();
                    }
                    else {
                        QFile file(path);
                        if (file.exists()) {
                            file.remove();
                        }
                    }
                    if (QFile::exists(path)) {
                        qWarning() << "Failed to delete" << path;
                    }
                }
            }
        }
    }
    else {
        qWarning() << "Please specify either --copy or --move.";
        return 1;
    }
}
