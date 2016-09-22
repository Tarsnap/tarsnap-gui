#ifndef FILETABLEMODEL_H
#define FILETABLEMODEL_H

#include "persistentmodel/archive.h"

#include <QAbstractTableModel>

class FileTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:

    enum TableColumns
    {
        FILE,
        MODIFIED,
        SIZE,
        USER,
        GROUP,
        MODE,
        LINKS
    };

    const int kTableColumnsCount = 7;

    FileTableModel(QObject *parent);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    void setArchive(ArchivePtr archive);
    void reset();

public slots:
    void setFiles(QVector<File> files);

private:
    QVector<File>    _files;
    ArchivePtr       _archive;
};

#endif // FILETABLEMODEL_H
