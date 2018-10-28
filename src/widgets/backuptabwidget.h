#ifndef BACKUPTABWIDGET_H
#define BACKUPTABWIDGET_H

#include "ui_backuptabwidget.h"

#include "backuplistwidget.h"
#include "backuptask.h"
#include "filepickerdialog.h"

#include <QEvent>
#include <QWidget>

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

    //! Does the Backup have a name?
    void validateBackupTab();

    // Temp for refactoring
    void temp_BackupListWidget(BackupListWidget *widget)
    {
        _ui_backupListWidget = widget;
    }

public slots:
    //! Open a file dialog to add (multiple) items.
    void browseForBackupItems();

    // Will be private after refactoring
    void updateBackupItemTotals(quint64 count, quint64 size);

signals:
    //! Begin tarsnap -c -f \<name\>
    void backupNow(BackupTaskPtr backupTask);

    //! Create a new job with the given urls and name.
    void morphBackupIntoJob(QList<QUrl> urls, QString name);

    //! We can make a new backup
    void backupTabValidStatus(bool valid);

    // Temp for refactoring
    void itemWithUrlAdded(QUrl url);

protected:
    //! Handles translation change of language.
    void changeEvent(QEvent *event);

private slots:
    void appendTimestampCheckBoxToggled(bool checked);
    void backupButtonClicked();
    void backupMorphIntoJobClicked();

private:
    Ui::BackupTabWidget _ui;
    FilePickerDialog    _filePickerDialog;

    QString _lastTimestamp;

    void updateUi();

    BackupListWidget *_ui_backupListWidget;
};

#endif // BACKUPTABWIDGET_H
