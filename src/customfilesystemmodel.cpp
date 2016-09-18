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
    if(role == Qt::CheckStateRole && index.column() == 0)
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
            // Return PartiallyChecked if any ancestor is checked.
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

QVariant CustomFileSystemModel::dataInternal(const QModelIndex &index) const
{
    if(_checklist.contains(index))
        return Qt::Checked;
    else if(_partialChecklist.contains(index))
        return Qt::PartiallyChecked;
    else
        return Qt::Unchecked;
}

void CustomFileSystemModel::setIndexCheckState(const QModelIndex &index,
                                               const Qt::CheckState state)
{
    if(dataInternal(index) != state)
        setDataInternal(index, state);
}

bool CustomFileSystemModel::hasAllSiblingsUnchecked(const QModelIndex &index)
{
    for(int i = 0; i < rowCount(index.parent()); i++)
    {
        QModelIndex sibling = index.sibling(i, index.column());
        if(sibling.isValid())
        {
            if(sibling == index)
                continue;
            if(dataInternal(sibling) != Qt::Unchecked)
                return false;
        }
    }
    return true;
}

bool CustomFileSystemModel::hasCheckedAncestor(const QModelIndex &index)
{
    QModelIndex ancestor = index.parent();
    while(ancestor.isValid())
    {
        if(dataInternal(ancestor) == Qt::Checked)
            return true;
        ancestor = ancestor.parent();
    }
    return false;
}

void CustomFileSystemModel::setUncheckedRecursive(const QModelIndex &index)
{
    if(isDir(index))
    {
        for(int i = 0; i < rowCount(index); i++)
        {
            QModelIndex child = index.child(i, index.column());
            if(child.isValid())
            {
                // Only alter a child if it was previously Checked or
                // PartiallyChecked.
                if(dataInternal(child) != Qt::Unchecked)
                {
                    setDataInternal(child, Qt::Unchecked);
                    if(isDir(child))
                        setUncheckedRecursive(child);
                }
            }
        }
    }
}

bool CustomFileSystemModel::setData(const QModelIndex &index,
                                    const QVariant &value, int role)
{
    if(role == Qt::CheckStateRole)
    {
        setDataInternal(index, value);

        QVector<int> selectionChangedRole;
        selectionChangedRole << SELECTION_CHANGED_ROLE;
        emit dataChanged(index, index, selectionChangedRole);
        return true;
    }
    return QFileSystemModel::setData(index, value, role);
}

void CustomFileSystemModel::setDataInternal(const QModelIndex &index,
                                            const QVariant &value)
{
    if(value == Qt::Checked)
    {
        _partialChecklist.remove(index);
        _checklist.insert(index);

        // Recursively set PartiallyChecked on all ancestors.
        QModelIndex parent         = index.parent();
        QModelIndex previousParent = index;
        while(parent.isValid())
        {
            // Set parent to be PartiallyChecked.
            setIndexCheckState(parent, Qt::PartiallyChecked);

            // Set any partially-selected siblings to be unchecked.
            for(int i = 0; i < rowCount(parent); i++)
            {
                QModelIndex child = parent.child(i, index.column());
                if(child.isValid())
                {
                    // Avoid unchecking previous parent.
                    if(child == previousParent)
                        continue;
                    if(dataInternal(child) == Qt::PartiallyChecked)
                        setIndexCheckState(child, Qt::Unchecked);
                }
            }

            // Ascend to higher level of ancestor.
            previousParent = parent;
            parent         = parent.parent();
        }

        // Set all descendents to be Unchecked.
        setUncheckedRecursive(index);
    }
    else if(value == Qt::PartiallyChecked)
    {
        _checklist.remove(index);
        _partialChecklist.insert(index);

        // Should the parent be partially checked?
        QModelIndex parent = index.parent();
        if(parent.isValid() && (dataInternal(parent) == Qt::Unchecked))
            setIndexCheckState(parent, Qt::PartiallyChecked);
    }
    else if(value == Qt::Unchecked)
    {
        _partialChecklist.remove(index);
        _checklist.remove(index);

        // Should the parent be unchecked?
        QModelIndex parent = index.parent();
        if(parent.isValid() && (dataInternal(parent) != Qt::Checked))
        {
            if(hasAllSiblingsUnchecked(index))
                setIndexCheckState(parent, Qt::Unchecked);
        }
    }
}

void CustomFileSystemModel::reset()
{
    _checklist.clear();
    _partialChecklist.clear();
}
