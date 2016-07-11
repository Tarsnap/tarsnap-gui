#ifndef ARCHIVEWIDGET_H
#define ARCHIVEWIDGET_H

#include "ui_archivewidget.h"
#include "filetablemodel.h"
#include "persistentmodel/archive.h"

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

protected:
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:
    Ui::ArchiveWidget      _ui;
    bool                   _useIECPrefixes;
    ArchivePtr             _archive;
    FileTableModel         _contentsModel;
    QSortFilterProxyModel  _proxyModel;
};

#endif // ARCHIVEWIDGET_H
