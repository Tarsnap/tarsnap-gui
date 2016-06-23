#ifndef CUSTOMFILESYSTEMMODEL_H
#define CUSTOMFILESYSTEMMODEL_H

#include <QFileSystemModel>
#include <QObject>
#include <QPersistentModelIndex>
#include <QSet>

#define SELECTION_CHANGED_ROLE Qt::UserRole + 100

/*!
 * \brief The CustomFileSystemModel is a QFileSystemModel which keeps track of
 * which files/directories have been (fully or partially) checked.
 *
 * Fully checked files or directories will be added to the backup list.
 * Partially checked directories indicate that some of their contents will be
 * added to the backup list.  If the user manually selects all of the contents
 * of a directory (by clicking on each file individually), that directory will
 * still be noted as "partially" checked, not "fully" checked.  A file cannot
 * be partially checked.
 */
class CustomFileSystemModel : public QFileSystemModel
{
public:
    //! Constructor.
    CustomFileSystemModel();
    ~CustomFileSystemModel();

    //! Returns a list of fully checked items.
    QList<QPersistentModelIndex> checkedIndexes();

    //! Returns metadata; used internally by the Qt layer.
    Qt::ItemFlags flags(const QModelIndex &index) const;
    //! Returns the Qt::CheckState or other data; used internally by the Qt
    //! layer.
    QVariant data(const QModelIndex &index, int role) const;

    //! Sets files or directories as being checked, partially checked, or
    //! unchecked.
    /*!
     * \param index: Indicates which file or directory's state to change.
     * \param value: Must be a Qt::CheckState, i.e. one of: {Qt::Unchecked,
     *               Qt::PartiallyChecked, Qt::Checked}.
     * \param role: Must be Qt::CheckStateRole.
     */
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    //! Clears the list of fully and partially checked files and dirs.
    void reset();

private:
    QSet<QPersistentModelIndex> _checklist;
    QSet<QPersistentModelIndex> _partialChecklist;

    // Sets the index to the desired state (if it is not already).
    void setIndexCheckState(const QModelIndex &index,
                            const Qt::CheckState state);
};

#endif // CUSTOMFILESYSTEMMODEL_H
