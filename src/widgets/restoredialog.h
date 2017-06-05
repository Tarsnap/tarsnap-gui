#ifndef RESTOREDIALOG_H
#define RESTOREDIALOG_H

#include "persistentmodel/archive.h"
#include "ui_restoredialog.h"

#include <QDialog>

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
    //! \param archive: the ArchivePtr to restore.
    //! \param parent: standard Qt parameter.
    explicit RestoreDialog(QWidget *parent, ArchivePtr archive,
                           QStringList files = QStringList());
    ~RestoreDialog();

    //! Returns options from the dialog window.
    ArchiveRestoreOptions getOptions();
    //! Display (or not) the "download uncompressed tar archive" option.
    void displayTarOption(bool display);

private slots:
    void optionBaseDirToggled(bool checked);
    void optionTarArchiveToggled(bool checked);
    void optionRestoreToggled(bool checked);
    void changeDir();
    void changeArchive();
    bool validate();

private:
    Ui::RestoreDialog _ui;
    ArchivePtr        _archive;
    QStringList       _files;
    QString           _downDir;

    void displayRestoreOption(bool display);
};

#endif // RESTOREDIALOG_H
