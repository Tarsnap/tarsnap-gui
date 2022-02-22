#ifndef JOBWIDGET_H
#define JOBWIDGET_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QList>
#include <QObject>
#include <QString>
#include <QWidget>
WARNINGS_ENABLE

#include "messages/archiveptr.h"
#include "messages/archiverestoreoptions.h"
#include "messages/jobptr.h"

/* Forward declaration(s). */
namespace Ui
{
class JobDetailsWidget;
}
class QEvent;
class QMenu;
class QTimer;

/*!
 * \ingroup widgets-specialized
 * \brief The JobDetailsWidget is a QWidget which displays detailed
 * information about a single Job.
 */
class JobDetailsWidget : public QWidget
{
    Q_OBJECT

#ifdef QT_TESTLIB_LIB
    friend class TestMainWindow;
    friend class TestJobsTabWidget;
#endif

public:
    //! Constructor.
    explicit JobDetailsWidget(QWidget *parent = nullptr);
    ~JobDetailsWidget() override;

    //! Returns the JobPtr associated with this widget.
    JobPtr job() const;
    //! Clears the current job, then sets a new job.
    void setJob(const JobPtr &job);

    //! Reload the IEC prefix preference and re-display number(s).
    void updateIEC();

public slots:
    //! Create (and save) a new job.
    void saveNew();
    //! Update the list of potential matching archives.
    //! \param archives the archives which match this job name.
    void updateMatchingArchives(const QList<ArchivePtr> &archives);

signals:
    //! The widget should be hidden.
    void collapse();
    //! A new job has been added.
    void jobAdded(JobPtr job);
    //! An archive belonging to this job should be inspected.
    void inspectJobArchive(ArchivePtr archive);
    //! Some archives belonging to this job should be deleted.
    void deleteJobArchives(const QList<ArchivePtr> &archives);
    //! The specified archive should be restored, using the
    //! user-selected options from the \ref RestoreDialog.
    void restoreJobArchive(ArchivePtr archive, ArchiveRestoreOptions options);
    //! Notify that we should create a new archive for this Job.
    void backupJob(JobPtr job);
    //! Notify whether we have enough information to create a new
    //! Job (i.e. job name and at least one file or directory).
    void enableSave(bool enable);
    //! Notify that we should look for archives matching this Job.
    //! \param jobPrefix prefix to match.
    void findMatchingArchives(const QString &jobPrefix);

protected:
    //! Handles translation change of language.
    void changeEvent(QEvent *event) override;

private slots:
    void updateDetails();
    void restoreButtonClicked();
    void backupButtonClicked();
    bool canSaveNew();
    void showArchiveListMenu();
    void fsEventReceived();
    void showJobPathsWarn();
    void verifyJob();

private:
    Ui::JobDetailsWidget *_ui;
    JobPtr                _job;
    bool                  _saveEnabled;
    QTimer               *_fsEventUpdate;

    QMenu *_archiveListMenu;

    void save();
    void updateKeyboardShortcutInfo();
};

#endif // JOBWIDGET_H
