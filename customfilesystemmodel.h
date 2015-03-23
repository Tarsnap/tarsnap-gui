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

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

private:
     QSet<QPersistentModelIndex> _checklist;
};

#endif // CUSTOMFILESYSTEMMODEL_H
