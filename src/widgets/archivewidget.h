#ifndef ARCHIVEWIDGET_H
#define ARCHIVEWIDGET_H

#include "persistentmodel/archive.h"
#include "utils.h"

#include <QWidget>

namespace Ui
{
class ArchiveWidget;
}

class ArchiveWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ArchiveWidget(QWidget *parent = 0);
    ~ArchiveWidget();

public slots:
    void setArchive(ArchivePtr archive);
    void updateDetails();

signals:
    void jobClicked(QString jobRef);

private:
    Ui::ArchiveWidget *_ui;
    bool               _useIECPrefixes;
    ArchivePtr         _archive;
};

#endif // ARCHIVEWIDGET_H
