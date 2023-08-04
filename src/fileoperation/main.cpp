#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("fileoperation");

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
        // Your code here
    }
    else {
        qWarning() << "Please specify either --copy or --move.";
        return 1;
    }
}
