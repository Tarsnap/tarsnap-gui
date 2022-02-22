#ifndef BACKUPTABWIDGET_H
#define BACKUPTABWIDGET_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QList>
#include <QObject>
#include <QString>
#include <QUrl>
#include <QUuid>
#include <QWidget>
WARNINGS_ENABLE

#include "messages/backuptaskdataptr.h"

/* Forward declaration(s). */
namespace Ui
{
class BackupTabWidget;
}
class BaseTask;
class FilePickerDialog;
class QEvent;

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
    ~BackupTabWidget() override;

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
    //! Reload the IEC prefix preference and re-display number(s).
    void updateIEC();

signals:
    //! Begin tarsnap -c -f \<name\>
    void backupNow(BackupTaskDataPtr backupTaskData);

    //! Create a new job with the given urls and name.
    void morphBackupIntoJob(const QList<QUrl> &urls, const QString &name);

    //! We can make a new backup
    void backupTabValidStatus(bool valid);

    //! We have a task to perform in the background.
    void taskRequested(BaseTask *task);
    //! We would like to cancel a task.
    void cancelTaskRequested(BaseTask *task, const QUuid &uuid);

protected:
    //! Handles translation change of language.
    void changeEvent(QEvent *event) override;

private slots:
    void appendTimestampCheckBoxToggled(bool checked);
    void backupButtonClicked();
    void backupMorphIntoJobClicked();
    void updateBackupItemTotals(quint64 count, quint64 size);

    // The FilePickerDialog finished.
    void processFPD(int res);

private:
    Ui::BackupTabWidget *_ui;
    FilePickerDialog    *_filePickerDialog;

    QString _lastTimestamp;

    quint64 _count;
    quint64 _size;

    void updateKeyboardShortcutInfo();
};

#endif // BACKUPTABWIDGET_H
