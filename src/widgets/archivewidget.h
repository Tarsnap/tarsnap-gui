#ifndef ARCHIVEWIDGET_H
#define ARCHIVEWIDGET_H

#include "ui_archivewidget.h"
#include "filetablemodel.h"
#include "persistentmodel/archive.h"

#include <QMenu>
#include <QWidget>

class ArchiveWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ArchiveWidget(QWidget *parent = nullptr);
    ~ArchiveWidget();

public slots:
    void setArchive(ArchivePtr archive);
    void updateDetails();

signals:
    void jobClicked(QString jobRef);
    void restoreArchive(ArchivePtr archive, ArchiveRestoreOptions options);

protected:
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent *event);

private slots:
    void showContextMenu(const QPoint &pos);
    void restoreFiles();

private:
    Ui::ArchiveWidget      _ui;
    bool                   _useIECPrefixes;
    ArchivePtr             _archive;
    FileTableModel         _contentsModel;
    QSortFilterProxyModel  _proxyModel;
    QMenu                  _fileMenu;
};

#endif // ARCHIVEWIDGET_H
