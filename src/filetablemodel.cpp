#include "filetablemodel.h"

FileTableModel::FileTableModel(QObject *parent) : QAbstractTableModel(parent)
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
    if(_archive)
        disconnect(_archive.data(), &Archive::fileList, this,
                   &FileTableModel::setFiles);
    reset();
    _archive = archive;
    if(_archive)
    {
        // ->getFileList() runs as a background task, so we can't
        // call it and block.  Instead, we essentially set up a
        // callback (::setFiles).
        connect(archive.data(), &Archive::fileList, this,
                &FileTableModel::setFiles);
        _archive->getFileList();
    }
}

void FileTableModel::setFiles(QVector<FileStat> files)
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
