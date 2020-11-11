#ifndef RESTOREDIALOG_H
#define RESTOREDIALOG_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QDialog>
#include <QObject>
#include <QString>
#include <QStringList>
WARNINGS_ENABLE

#include "messages/archiveptr.h"

/* Forward declaration(s). */
namespace Ui
{
class RestoreDialog;
}
class QWidget;
struct ArchiveRestoreOptions;

/*!
 * \ingroup widgets-specialized
 * \brief The RestoreDialog is a QDialog which asks the user where to restore
 * an archive, and whether to overwrite files.
 */
class RestoreDialog : public QDialog
{
    Q_OBJECT

public:
    //! Constructor.
    //! \param archive the ArchivePtr to restore.
    //! \param parent standard Qt parameter.
    //! \param files list of files to restore.  If empty, restore all files
    //!               in this archive.
    explicit RestoreDialog(QWidget *parent, ArchivePtr archive,
                           const QStringList &files = QStringList());
    ~RestoreDialog() override;

    //! Returns options from the dialog window.
    ArchiveRestoreOptions getOptions();
    //! Display (or not) the "download uncompressed tar archive" option.
    void displayTarOption(bool display);
    //! Returns current archive.
    ArchivePtr archive() const;

private slots:
    void optionBaseDirToggled(bool checked);
    void optionTarArchiveToggled(bool checked);
    void optionRestoreToggled(bool checked);
    void changeDir();
    void changeArchive();
    bool validate();

private:
    Ui::RestoreDialog *_ui;
    ArchivePtr         _archive;
    QStringList        _files;
    QString            _downDir;

    void displayRestoreOption(bool display);
};

#endif // RESTOREDIALOG_H
