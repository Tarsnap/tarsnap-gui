#ifndef FILETABLEMODEL_H
#define FILETABLEMODEL_H

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

    struct File {
        QString name;
        QString modified;
        quint64 size;
        QString user;
        QString group;
        QString mode;
        quint64 links;
    };

    FileTableModel(QObject *parent);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    void setFiles(const QString &listing);
    void reset();

private:
    QVector<File>    _files;
    QVector<QString> _columns;
};

#endif // FILETABLEMODEL_H
