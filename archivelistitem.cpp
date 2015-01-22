#include "archivelistitem.h"

ArchiveListItem::ArchiveListItem(QObject *parent): _widget(new QWidget())
{
    _ui.setupUi(_widget);
}

ArchiveListItem::~ArchiveListItem()
{

}

