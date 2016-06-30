#include "filetablemodel.h"


#include "debug.h"

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

void FileTableModel::setFiles(const QString &listing)
{
    beginResetModel();
    _files.clear();
    foreach(QString line, listing.split('\n', QString::SkipEmptyParts))
    {
        QRegExp fileRX("^(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+\\s+\\S+\\s+\\S+)\\s+(.+)$");
        if(-1 != fileRX.indexIn(line))
        {
            QStringList fileListing = fileRX.capturedTexts();
            fileListing.removeFirst();
            File file;
            file.mode  = fileListing[0];
            file.links = fileListing[1].toULongLong();
            file.user  = fileListing[2];
            file.group = fileListing[3];
            file.size  = fileListing[4].toULongLong();
            file.modified = fileListing[5];
            file.name     = fileListing[6];
            _files.append(file);
        }
    }
    endResetModel();
}

void FileTableModel::reset()
{
    beginResetModel();
    _files.clear();
    endResetModel();
}
