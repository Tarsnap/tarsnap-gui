#include "customfilesystemmodel.h"

WARNINGS_DISABLE
#include <QFlags>
#include <QModelIndex>
#include <QObject>
#include <QPersistentModelIndex>
#include <QVector>
WARNINGS_ENABLE

CustomFileSystemModel::CustomFileSystemModel()
{
}

QList<QPersistentModelIndex> CustomFileSystemModel::checkedIndexes()
{
    return _checklist.values();
}

Qt::ItemFlags CustomFileSystemModel::flags(const QModelIndex &idx) const
{
    return QFileSystemModel::flags(idx) | Qt::ItemIsUserCheckable;
}

QVariant CustomFileSystemModel::data(const QModelIndex &idx, int role) const
{
    if(role == Qt::CheckStateRole && idx.column() == 0)
    {
        if(_checklist.contains(idx))
        {
            return Qt::Checked;
        }
        else if(_partialChecklist.contains(idx))
        {
            return Qt::PartiallyChecked;
        }
        else
        {
            // Returns PartiallyChecked if any ancestor is checked.
            QModelIndex parent = idx.parent();
            while(parent.isValid())
            {
                if(_checklist.contains(parent))
                    return Qt::PartiallyChecked;
                parent = parent.parent();
            }
        }
        return Qt::Unchecked;
    }
    return QFileSystemModel::data(idx, role);
}

QVariant CustomFileSystemModel::dataInternal(const QModelIndex &idx) const
{
    if(_checklist.contains(idx))
        return Qt::Checked;
    else if(_partialChecklist.contains(idx))
        return Qt::PartiallyChecked;
    else
        return Qt::Unchecked;
}

void CustomFileSystemModel::setIndexCheckState(const QModelIndex   &idx,
                                               const Qt::CheckState state)
{
    if(dataInternal(idx) != state)
        setDataInternal(idx, state);
}

bool CustomFileSystemModel::hasAllSiblingsUnchecked(const QModelIndex &idx)
{
    for(int i = 0; i < rowCount(idx.parent()); i++)
    {
        QModelIndex sibling = idx.sibling(i, idx.column());
        if(sibling.isValid())
        {
            if(sibling == idx)
                continue;
            if(dataInternal(sibling) != Qt::Unchecked)
                return false;
        }
    }
    return true;
}

bool CustomFileSystemModel::hasCheckedAncestor(const QModelIndex &idx)
{
    QModelIndex ancestor = idx.parent();
    while(ancestor.isValid())
    {
        if(dataInternal(ancestor) == Qt::Checked)
            return true;
        ancestor = ancestor.parent();
    }
    return false;
}

void CustomFileSystemModel::setUncheckedRecursive(const QModelIndex &idx)
{
    if(isDir(idx))
    {
        for(int i = 0; i < rowCount(idx); i++)
        {
            QModelIndex child = index(i, idx.column(), idx);
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

bool CustomFileSystemModel::setData(const QModelIndex &idx,
                                    const QVariant &value, int role)
{
    if(role == Qt::CheckStateRole)
    {
        setDataInternal(idx, value);

        QVector<int> selectionChangedRole;
        selectionChangedRole << SELECTION_CHANGED_ROLE;
        emit dataChanged(idx, idx, selectionChangedRole);
        return true;
    }
    return QFileSystemModel::setData(idx, value, role);
}

void CustomFileSystemModel::setDataInternal(const QModelIndex &idx,
                                            const QVariant    &value)
{
    if(value == Qt::Checked)
    {
        _partialChecklist.remove(idx);
        _checklist.insert(idx);

        // Recursively set PartiallyChecked on all ancestors.
        QModelIndex parent         = idx.parent();
        QModelIndex previousParent = idx;
        while(parent.isValid())
        {
            // Set parent to be PartiallyChecked.
            setIndexCheckState(parent, Qt::PartiallyChecked);

            // Set any partially-selected siblings to be unchecked.
            for(int i = 0; i < rowCount(parent); i++)
            {
                QModelIndex child = index(i, idx.column(), parent);
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
        setUncheckedRecursive(idx);
    }
    else if(value == Qt::PartiallyChecked)
    {
        _checklist.remove(idx);
        _partialChecklist.insert(idx);

        // Should the parent be partially checked?
        QModelIndex parent = idx.parent();
        if(parent.isValid() && (dataInternal(parent) == Qt::Unchecked))
            setIndexCheckState(parent, Qt::PartiallyChecked);
    }
    else if(value == Qt::Unchecked)
    {
        _partialChecklist.remove(idx);
        _checklist.remove(idx);

        // Should the parent be unchecked?
        QModelIndex parent = idx.parent();
        if(parent.isValid() && (dataInternal(parent) != Qt::Checked))
        {
            if(hasAllSiblingsUnchecked(idx))
                setIndexCheckState(parent, Qt::Unchecked);
        }
    }
}

void CustomFileSystemModel::reset()
{
    _checklist.clear();
    _partialChecklist.clear();
}

bool CustomFileSystemModel::needToReadSubdirs(const QString &dirname)
{
    bool        loadingMore = false;
    QModelIndex dir         = index(dirname);
    // QFileSystemModel::canFetchMore(dir) can apparently lie about whether
    // its has finished loading a directory.  Alternatively, it might be
    // designed to merely report that a directory has been *queued* to be
    // read, but not actually read yet, and the Qt docs were simply written
    // badly.
    //
    // Either way, we must treat every directory which contains 0 items
    // has not finished being read from disk.  This is suggested by the
    // API docs:
    //     "Calls to rowCount() will return 0 until the model populates a
    //     directory."
    //     https://doc.qt.io/qt-5/qfilesystemmodel.html#caching-and-performance
    if(isDir(dir) && rowCount(dir) == 0)
    {
        fetchMore(dir);
        loadingMore = true;
    }
    for(int i = 0; i < rowCount(dir); i++)
    {
        QModelIndex child = index(i, dir.column(), dir);
        if(isDir(child))
        {
            if(rowCount(child) == 0)
            {
                fetchMore(child);
                loadingMore = true;
            }
            if(needToReadSubdirs(filePath(child)))
            {
                loadingMore = true;
            }
        }
    }
    return loadingMore;
}
