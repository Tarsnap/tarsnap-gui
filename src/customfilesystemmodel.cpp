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
        setData(index, state, Qt::CheckStateRole);
}

bool CustomFileSystemModel::hasCheckedSibling(const QModelIndex &index)
{
    for(int i = 0; i < rowCount(index.parent()); i++)
    {
        QModelIndex sibling = index.sibling(i, index.column());
        if(sibling.isValid())
        {
            if(sibling == index)
                continue;
            if(dataInternal(sibling) != Qt::Unchecked)
                return true;
        }
    }
    return false;
}

bool CustomFileSystemModel::hasCheckedAncestor(const QModelIndex &index)
{
    QModelIndex ancestor = index.parent();
    while(ancestor.isValid())
    {
        if(ancestor.data(Qt::CheckStateRole) == Qt::Checked)
            return true;
        ancestor = ancestor.parent();
    }
    return false;
}

QList<QModelIndex> CustomFileSystemModel::getFakePCRecursive(
        const QModelIndex &index)
{
    QList<QModelIndex> indices;
    if(isDir(index))
    {
        for(int i = 0; i < rowCount(index); i++)
        {
            QModelIndex child = index.child(i, index.column());
            if(child.isValid())
            {
                if((dataInternal(child) == Qt::Unchecked) &&
                        (data(child, Qt::CheckStateRole) ==
                         Qt::PartiallyChecked))
                {
                    indices << child;
                    if(isDir(child))
                        indices << getFakePCRecursive(child);
                }
            }
        }
    }
    return indices;
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
                    setData(child, Qt::Unchecked, Qt::CheckStateRole);
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
    QVector<int> selectionChangedRole;
    selectionChangedRole << SELECTION_CHANGED_ROLE;
    if(role == Qt::CheckStateRole)
    {
        if(value == Qt::Checked)
        {
            _partialChecklist.remove(index);
            _checklist.insert(index);
            emit dataChanged(index, index, selectionChangedRole);

            // Recursively set PartiallyChecked on all ancestors, and make an
            // ordered list of checked ancestors.
            QModelIndex        parent = index.parent();
            QList<QModelIndex> checkedAncestors;
            while(parent.isValid())
            {
                if((dataInternal(parent) == Qt::Checked) ||
                   hasCheckedAncestor(parent))
                    checkedAncestors << parent;

                // Set parent to be PartiallyChecked.
                setIndexCheckState(parent, Qt::PartiallyChecked);

                // Ascend to higher level of ancestor.
                parent = parent.parent();
            }

            // Uncheck partially-selected siblings.  Must be done after
            // unchecking ancestors (above), otherwise the emit() generated by
            // an unchecked sibling will still report PartiallyChecked.
            parent                     = index.parent();
            QModelIndex previousParent = index;
            for(int j = 0; j < checkedAncestors.count(); j++)
            {
                parent = checkedAncestors.at(j);
                // Set any partially-selected siblings to be unchecked.
                for(int i = 0; i < rowCount(parent); i++)
                {
                    QModelIndex child = parent.child(i, index.column());
                    if(child.isValid())
                    {
                        // Avoid unchecking previous parent.
                        if(child == previousParent)
                            continue;
                        if(data(child, Qt::CheckStateRole) == Qt::PartiallyChecked)
                            setIndexCheckState(child, Qt::Unchecked);
                    }
                }
                // Ascend to higher level of ancestor.
                previousParent = parent;
                parent         = parent.parent();
            }

            // Check descendants
            setUncheckedRecursive(index);
        }
        else if(value == Qt::PartiallyChecked)
        {
            if(_checklist.remove(index))
                emit dataChanged(index, index, selectionChangedRole);
            _partialChecklist.insert(index);
            QModelIndex parent = index.parent();
            if(parent.isValid() &&
               (parent.data(Qt::CheckStateRole) == Qt::Unchecked))
                setIndexCheckState(parent, Qt::PartiallyChecked);
        }
        else if(value == Qt::Unchecked)
        {
            // Get list of files which need manually emitting.  Must be
            // done before modifying the _checklist.
            QList<QModelIndex> fakeDescendents = getFakePCRecursive(index);

            _partialChecklist.remove(index);
            if(_checklist.remove(index))
                emit dataChanged(index, index, selectionChangedRole);

            // Check ancestor
            QModelIndex parent = index.parent();
            if(parent.isValid() &&
                    (parent.data(Qt::CheckStateRole) != Qt::Checked))
            {
                if(hasCheckedSibling(index))
                    setIndexCheckState(parent, Qt::PartiallyChecked);
                else
                    setIndexCheckState(parent, Qt::Unchecked);
            }

            // Emit data for descendents which falsely reported PC.
            for (int i = 0; i < fakeDescendents.count(); i++) {
                QModelIndex descendent = fakeDescendents.at(i);
                emit dataChanged(descendent, descendent, selectionChangedRole);
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
