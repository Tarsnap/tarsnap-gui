#ifndef FILETABLEMODEL_H
#define FILETABLEMODEL_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QAbstractTableModel>
#include <QModelIndex>
#include <QObject>
#include <QVariant>
#include <QVector>
#include <Qt>
WARNINGS_ENABLE

#include "messages/archivefilestat.h"
#include "messages/archiveptr.h"

/* Forward declaration(s). */
class BaseTask;
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
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    //! Returns the number of columns (const \c kTableColumnsCount).
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    //! Returns file information for an index.
    //! \param index which file (row), and what type of information (column).
    //! \param role should be \c Qt::DisplayRole.
    QVariant data(const QModelIndex &index,
                  int                role = Qt::DisplayRole) const override;

    //! Returns the text for a header field.
    //! \param section which row or column number.
    //! \param orientation \c Qt::Vertical or \c Qt::Horizontal.
    //! \param role should be \c Qt::DisplayRole.
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    //! Sets the archive from which this object should load the file list.
    //! This spawns a \ref BaseTask, emitted via \ref taskRequested, which
    //! must be deleted by external code.
    void setArchive(ArchivePtr archive);

    //! Clears the stored information about files.
    void reset();

public slots:
    //! Sets the list of files to be stored in this object.
    void setFiles(const QVector<FileStat> &files);

signals:
    //! We have a task to perform in the background.
    void taskRequested(BaseTask *task);

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
