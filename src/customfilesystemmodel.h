#ifndef CUSTOMFILESYSTEMMODEL_H
#define CUSTOMFILESYSTEMMODEL_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QFileSystemModel>
#include <QList>
#include <QSet>
#include <QString>
#include <QVariant>
#include <Qt>
WARNINGS_ENABLE

/* Forward declaration(s). */
class QModelIndex;
class QPersistentModelIndex;

#define SELECTION_CHANGED_ROLE Qt::UserRole + 100

/*!
 * \ingroup misc
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

    //! Returns a list of fully checked items.
    QList<QPersistentModelIndex> checkedIndexes();

    //! Returns metadata; used internally by the Qt layer.
    Qt::ItemFlags flags(const QModelIndex &idx) const override;
    //! Returns the Qt::CheckState or other data; used internally by the Qt
    //! layer.
    QVariant data(const QModelIndex &idx, int role) const override;

    //! Sets files or directories as being checked, partially checked, or
    //! unchecked.
    /*!
     * \param idx Indicates which file or directory's state to change.
     * \param value Must be a Qt::CheckState, i.e. one of: {Qt::Unchecked,
     *               Qt::PartiallyChecked, Qt::Checked}.
     * \param role Must be Qt::CheckStateRole.
     */
    bool setData(const QModelIndex &idx, const QVariant &value,
                 int role) override;

    //! Clears the list of fully and partially checked files and dirs.
    void reset();

    //! Has QFileSystemModel (separate thread) finished caching this directory?
    bool needToReadSubdirs(const QString &dirname);

private:
    QSet<QPersistentModelIndex> _checklist;
    QSet<QPersistentModelIndex> _partialChecklist;

    // Returns the actual data (i.e. without fakery with PartiallyChecked).
    QVariant dataInternal(const QModelIndex &idx) const;

    // Updates the index Checked state without generating an emit.
    void setDataInternal(const QModelIndex &idx, const QVariant &value);

    // Sets the index to the desired state (if it is not already).
    void setIndexCheckState(const QModelIndex &idx, const Qt::CheckState state);

    // Checks that any&all siblings are unchecked.
    bool hasAllSiblingsUnchecked(const QModelIndex &idx);

    // Searches for a checked ancestor.
    bool hasCheckedAncestor(const QModelIndex &idx);

    // Recursively sets previously-checked descendents to be unchecked.
    void setUncheckedRecursive(const QModelIndex &idx);
};

#endif // CUSTOMFILESYSTEMMODEL_H
