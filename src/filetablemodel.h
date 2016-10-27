#ifndef FILETABLEMODEL_H
#define FILETABLEMODEL_H

#include "persistentmodel/archive.h"

#include <QAbstractTableModel>

/*!
 * \ingroup data
 * \brief The FileTableModel is a QAbstractTableModel which stores
 * a list of files in an Archive.
 */
class FileTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    //! Constructor
    FileTableModel(QObject *parent);

    //! Returns the number of files in the table.
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    //! Returns the number of columns (a fixed value).
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    //! Returns some information about a file.
    //! \param index: which file (row), and what type of information (column).
    //! \param role: should be \c Qt::DisplayRole.
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    //! Returns the text for a header field.
    //! \param section: which row or column number.
    //! \param orientation: \c Qt::Vertical or \c Qt::Horizontal.
    //! \param role: should be \c Qt::DisplayRole.
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    //! Sets the archive from which this object should load the file list.
    void setArchive(ArchivePtr archive);

    //! Clears the stored information about files.
    void reset();

public slots:
    //! Sets the list of files to be stored in thie object.
    void setFiles(QVector<File> files);

private:
    QVector<File> _files;
    ArchivePtr    _archive;

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
};

#endif // FILETABLEMODEL_H
