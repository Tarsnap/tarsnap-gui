#include "browselistwidget.h"
#include "ui_archiveitemwidget.h"

BrowseListWidget::BrowseListWidget(QWidget *parent):
    QListWidget(parent)
{
    Ui::ArchiveItemWidget archiveItemUi;
    for(int i = 0; i < 10; i++)
    {
        QListWidgetItem *item = new QListWidgetItem;
        QWidget *widget = new QWidget;
        archiveItemUi.setupUi(widget);
        insertItem(i, item);
        setItemWidget(item, widget);
    }
}

BrowseListWidget::~BrowseListWidget()
{

}

void BrowseListWidget::addArchives(QList<QSharedPointer<Archive> > archives)
{
    _archives = archives;
}

