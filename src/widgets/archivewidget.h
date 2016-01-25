#ifndef ARCHIVEWIDGET_H
#define ARCHIVEWIDGET_H

#include "utils.h"
#include "persistentmodel/archive.h"

#include <QWidget>

namespace Ui {
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
    Ui::ArchiveWidget  *_ui;
    bool                _useSIPrefixes;
    ArchivePtr          _archive;
};

#endif // ARCHIVEWIDGET_H
