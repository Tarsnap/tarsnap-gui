#include "customfilesystemmodel.h"

CustomFileSystemModel::CustomFileSystemModel()
{
}

CustomFileSystemModel::~CustomFileSystemModel()
{

}

QList<QPersistentModelIndex> CustomFileSystemModel::checkedIndexes()
{
    return _checklist.toList();
}

Qt::ItemFlags CustomFileSystemModel::flags(const QModelIndex &index) const
{
    return QFileSystemModel::flags(index) | Qt::ItemIsUserCheckable;
}

QVariant CustomFileSystemModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::CheckStateRole && index.column() == 0)
    {
        if(_checklist.contains(index))
        {
            return Qt::Checked;
        }
        else if(_partialChecklist.contains(index))
        {
            return Qt::PartiallyChecked;
        }
        else
        {
            QModelIndex parent = index.parent();
            while(parent.isValid())
            {
                if(_checklist.contains(parent))
                    return Qt::PartiallyChecked;
                parent = parent.parent();
            }
        }
        return Qt::Unchecked;
    }
    return QFileSystemModel::data(index, role);
}

bool CustomFileSystemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole)
    {
        if (value == Qt::Checked)
        {
            _partialChecklist.remove(index);
            _checklist.insert(index);
            if(index.parent().isValid())
            {
                if(index.parent().data(Qt::CheckStateRole) == Qt::Checked)
                {
                    for(int i = 0; i < rowCount(index.parent()); i++)
                    {
                        if(index.sibling(i, index.column()).isValid())
                        {
                            if(index.sibling(i,index.column()) == index)
                                continue;
                            if(data(index.sibling(i, index.column()), Qt::CheckStateRole) == Qt::PartiallyChecked)
                                setData(index.sibling(i, index.column()), Qt::Unchecked, Qt::CheckStateRole);
                        }
                    }
                }
                if(index.parent().data(Qt::CheckStateRole) != Qt::PartiallyChecked)
                    setData(index.parent(), Qt::PartiallyChecked, Qt::CheckStateRole);
            }
            if(isDir(index))
            {
                for(int i = 0; i < rowCount(index); i++)
                {
                    if(index.child(i, index.column()).isValid())
                        setData(index.child(i, index.column()), Qt::PartiallyChecked, Qt::CheckStateRole);
                }
            }
        }
        else if(value == Qt::PartiallyChecked)
        {
            _checklist.remove(index);
            _partialChecklist.insert(index);
            if(index.parent().isValid() && (index.parent().data(Qt::CheckStateRole) == Qt::Unchecked))
                setData(index.parent(), Qt::PartiallyChecked, Qt::CheckStateRole);
        }
        else if (value == Qt::Unchecked)
        {
            _checklist.remove(index);
            _partialChecklist.remove(index);

            if(isDir(index))
            {
                for(int i = 0; i < rowCount(index); i++)
                {
                    if(index.child(i, index.column()).isValid())
                        setData(index.child(i, index.column()), Qt::Unchecked, Qt::CheckStateRole);
                }
            }

            if(index.parent().isValid())
            {
                bool foundOne = false;
                for(int i = 0; i < rowCount(index.parent()); i++)
                {
                    if(index.sibling(i, index.column()).isValid())
                    {
                        if(index.sibling(i,index.column()) == index)
                            continue;
                        if(data(index.sibling(i, index.column()), Qt::CheckStateRole) != Qt::Unchecked)
                            foundOne = true;
                    }
                }
                if(foundOne)
                {
                    if(index.parent().data(Qt::CheckStateRole) != Qt::PartiallyChecked)
                        setData(index.parent(), Qt::PartiallyChecked, Qt::CheckStateRole);
                }
                else
                {
                    if(index.parent().data(Qt::CheckStateRole) != Qt::Unchecked)
                        setData(index.parent(), Qt::Unchecked, Qt::CheckStateRole);
                }
            }
        }
        QVector<int> roles;
        roles << Qt::CheckStateRole;
        emit dataChanged(index, index, roles);
        return true;
    }
    return QFileSystemModel::setData(index, value, role);
}

void CustomFileSystemModel::reset()
{
    _checklist.clear();
    _partialChecklist.clear();
}

