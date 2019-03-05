#ifndef BACKUPTABWIDGET_H
#define BACKUPTABWIDGET_H

#include <QEvent>
#include <QWidget>

#include "backuplistwidget.h"
#include "backuptask.h"
#include "filepickerdialog.h"

namespace Ui
{
class BackupTabWidget;
}

/*!
 * \ingroup widgets-main
 * \brief The BackupTabWidget is a QWidget which shows the list of jobs.
 */
class BackupTabWidget : public QWidget
{
    Q_OBJECT

#ifdef QT_TESTLIB_LIB
    friend class TestBackupTabWidget;
    friend class TestMainWindow;
#endif

public:
    //! Constructor.
    explicit BackupTabWidget(QWidget *parent = nullptr);
    ~BackupTabWidget();

    //! Does the Backup have a name?
    void validateBackupTab();

public slots:
    //! Open a file dialog to add (multiple) items.
    void browseForBackupItems();
    //! Open a file dialog to add (multiple) files (but not directories!).
    void addFiles();
    //! Open a file dialog to add (multiple) directories (but not files!).
    void addDirectory();
    //! Remove items from the backup list.
    void clearList();

signals:
    //! Begin tarsnap -c -f \<name\>
    void backupNow(BackupTaskPtr backupTask);

    //! Create a new job with the given urls and name.
    void morphBackupIntoJob(QList<QUrl> urls, QString name);

    //! We can make a new backup
    void backupTabValidStatus(bool valid);

protected:
    //! Handles translation change of language.
    void changeEvent(QEvent *event);

private slots:
    void appendTimestampCheckBoxToggled(bool checked);
    void backupButtonClicked();
    void backupMorphIntoJobClicked();
    void updateBackupItemTotals(quint64 count, quint64 size);

private:
    Ui::BackupTabWidget *_ui;
    FilePickerDialog     _filePickerDialog;

    QString _lastTimestamp;

    void updateUi();
};

#endif // BACKUPTABWIDGET_H
