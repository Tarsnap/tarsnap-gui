#include "filetablemodel.h"

WARNINGS_DISABLE
#include <QAbstractTableModel>
#include <QVariant>
#include <QVector>
WARNINGS_ENABLE

#include "messages/archivefilestat.h"

#include "parsearchivelistingtask.h"
#include "persistentmodel/archive.h"

FileTableModel::FileTableModel(QObject *parent)
    : QAbstractTableModel(parent), _parseTask(nullptr)
{
}

int FileTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return _files.count();
}

int FileTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return kTableColumnsCount;
}

QVariant FileTableModel::data(const QModelIndex &index, int role) const
{
    if(role == Qt::DisplayRole)
    {
        switch(index.column())
        {
        case TableColumns::FILE:
            return _files[index.row()].name;
        case TableColumns::MODIFIED:
            return _files[index.row()].modified;
        case TableColumns::SIZE:
            return _files[index.row()].size;
        case TableColumns::USER:
            return _files[index.row()].user;
        case TableColumns::GROUP:
            return _files[index.row()].group;
        case TableColumns::MODE:
            return _files[index.row()].mode;
        case TableColumns::LINKS:
            return _files[index.row()].links;
        }
    }
    return QVariant();
}

QVariant FileTableModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{
    if(role != Qt::DisplayRole)
        return QVariant();
    if(orientation == Qt::Vertical)
        return QString("%1").arg(section + 1);
    if(orientation == Qt::Horizontal)
    {
        switch(section)
        {
        case TableColumns::FILE:
            return tr("FILE");
        case TableColumns::MODIFIED:
            return tr("DATE MODIFIED");
        case TableColumns::SIZE:
            return tr("SIZE");
        case TableColumns::USER:
            return tr("USER");
        case TableColumns::GROUP:
            return tr("GROUP");
        case TableColumns::MODE:
            return tr("MODE");
        case TableColumns::LINKS:
            return tr("LINKS");
        }
    }
    return QVariant();
}

void FileTableModel::setArchive(ArchivePtr archive)
{
    // Disable previous connection (if it exists).
    if(_parseTask)
        disconnect(_parseTask, &ParseArchiveListingTask::result, this,
                   &FileTableModel::setFiles);
    reset();
    _archive = archive;
    if(_archive)
    {
        // Prepare a background thread to parse the Archive's saved contents.
        ParseArchiveListingTask *parseTask =
            new ParseArchiveListingTask(archive->contents());
        connect(parseTask, &ParseArchiveListingTask::result, this,
                &FileTableModel::setFiles);
        emit taskRequested(parseTask);
    }
}

void FileTableModel::setFiles(const QVector<FileStat> &files)
{
    // This indicates that our internal data is changing.
    beginResetModel();
    _files = files;
    // We finished changing internal data; any views using this
    // model will refresh.
    endResetModel();
}

void FileTableModel::reset()
{
    beginResetModel();
    _files.clear();
    endResetModel();
}
