#ifndef FILETABLEMODEL_H
#define FILETABLEMODEL_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QAbstractTableModel>
#include <QObject>
#include <QVariant>
#include <QVector>
WARNINGS_ENABLE

#include "archivefilestat.h"
#include "persistentmodel/archive.h"

/* Forward declaration. */
class ParseArchiveListingTask;

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
    explicit FileTableModel(QObject *parent);

    //! Returns the number of files in the table.
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    //! Returns the number of columns (const \c kTableColumnsCount).
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    //! Returns file information for an index.
    //! \param index which file (row), and what type of information (column).
    //! \param role should be \c Qt::DisplayRole.
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    //! Returns the text for a header field.
    //! \param section which row or column number.
    //! \param orientation \c Qt::Vertical or \c Qt::Horizontal.
    //! \param role should be \c Qt::DisplayRole.
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    //! Sets the archive from which this object should load the file list.
    void setArchive(ArchivePtr archive);

    //! Clears the stored information about files.
    void reset();

public slots:
    //! Sets the list of files to be stored in this object.
    void setFiles(QVector<FileStat> files);

private:
    QVector<FileStat> _files;
    ArchivePtr        _archive;

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

    ParseArchiveListingTask *_parseTask;
};

#endif // FILETABLEMODEL_H
