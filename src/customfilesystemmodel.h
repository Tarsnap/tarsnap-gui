#ifndef CUSTOMFILESYSTEMMODEL_H
#define CUSTOMFILESYSTEMMODEL_H

#include <QObject>
#include <QFileSystemModel>
#include <QPersistentModelIndex>
#include <QSet>

class CustomFileSystemModel : public QFileSystemModel
{
public:
    CustomFileSystemModel();
    ~CustomFileSystemModel();

    QList<QPersistentModelIndex> checkedIndexes();

    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant      data(const QModelIndex &index, int role) const;
    bool          setData(const QModelIndex &index, const QVariant &value, int role);
    void          reset();

private:
     QSet<QPersistentModelIndex> _checklist;
     QSet<QPersistentModelIndex> _partialChecklist;
};

#endif // CUSTOMFILESYSTEMMODEL_H
