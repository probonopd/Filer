#ifndef FILEMANAGERMAINWINDOW_H
#define FILEMANAGERMAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QTreeView>
#include <QListView>
#include <QStatusBar>
#include <QList>
#include <QStackedWidget>
#include "CustomFileSystemModel.h"
#include "CustomListView.h"

class FileManagerMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static QList<FileManagerMainWindow *> & instances();
    QString getPath() const;
    

    bool instanceExists(const QString &directory);

    FileManagerMainWindow(QWidget *parent = nullptr, const QString &initialDirectory = "/");

    ~FileManagerMainWindow();

    QString directory() const;
    void setDirectory(const QString &directory);

    void showAboutBox();

    void showTreeView();
    void showIconView();
    void showHideStatusBar();

    void refresh();

    CustomFileSystemModel *m_fileSystemModel;

    bool m_is_first_instance = false;

    QWidget* getCurrentView() const;

    FileManagerMainWindow* getInstanceForDirectory(const QString &directory);

public slots:
    void open(const QString &filePath);
    void openWith(const QString &filePath);
    void openFolderInNewWindow(const QString &rootPath);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void renameSelectedItem();
    void resizeEvent(QResizeEvent *event);

    void moveEvent(QMoveEvent *event);
    void dropEvent(QDropEvent *event);

private:
    QStackedWidget *m_stackedWidget;

    QString m_currentDir;
    
    QTreeView *m_treeView;
    CustomListView *m_iconView;
    QItemSelectionModel *m_selectionModel;
    QMenuBar *m_menuBar;

    QStatusBar *m_statusBar;
    QAction *m_treeViewAction;
    QAction *m_iconViewAction;

    QAction *m_openAction;
    QAction *m_openWithAction;
    QAction *m_renameAction;

    QAction *m_showHideStatusBarAction;

    QStringList readFilenamesFromHiddenFile(const QString &filePath);

    void createMenus();

    void selectAll();
    void updateStatusBar();
    void copySelectedItems(const QString &destinationPath);
    void moveSelectedItems(const QString &destinationPath);
    void onItemDropped(const QModelIndex &targetIndex);
    void enableDropOnFolder(const QModelIndex &index);
    void updateMenus();

    void saveWindowGeometry();
};

#endif // FILEMANAGERMAINWINDOW_H
