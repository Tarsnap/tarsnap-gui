#include "filetablemodel.h"

FileTableModel::FileTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    _columns << tr("FILE") << tr("DATE MODIFIED") << tr("SIZE") << tr("USER")
             << tr("GROUP") << tr("MODE") << tr("LINKS");
}

int FileTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return _files.count();
}

int FileTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return _columns.count();
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

QVariant FileTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role != Qt::DisplayRole)
        return QVariant();
    if(orientation == Qt::Vertical)
        return QString("%1").arg(section + 1);
    if(orientation == Qt::Horizontal)
        return _columns[section];
    return QVariant();
}

void FileTableModel::setArchive(ArchivePtr archive)
{
    if(_archive)
        disconnect(_archive.data(), &Archive::fileList, this, &FileTableModel::setFiles);
    reset();
    _archive = archive;
    if(_archive)
    {
        connect(archive.data(), &Archive::fileList, this, &FileTableModel::setFiles);
        _archive->getFileList();
    }
}

void FileTableModel::setFiles(QVector<File> files)
{
    beginResetModel();
    _files = files;
    endResetModel();
}

void FileTableModel::reset()
{
    beginResetModel();
    _files.clear();
    endResetModel();
}
