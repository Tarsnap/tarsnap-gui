#ifndef ARCHIVESTABWIDGET_H
#define ARCHIVESTABWIDGET_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QList>
#include <QObject>
#include <QString>
#include <QWidget>
WARNINGS_ENABLE

#include "messages/archiveptr.h"
#include "messages/archiverestoreoptions.h"

/* Forward declaration(s). */
namespace Ui
{
class ArchivesTabWidget;
}
class BaseTask;
class QEvent;
class QMenu;

/*!
 * \ingroup widgets-main
 * \brief The ArchivesTabWidget is a QWidget which shows the Archives tab.
 */
class ArchivesTabWidget : public QWidget
{
    Q_OBJECT

#ifdef QT_TESTLIB_LIB
    friend class TestArchivesTabWidget;
    friend class TestMainWindow;
#endif

public:
    //! Constructor.
    explicit ArchivesTabWidget(QWidget *parent = nullptr);
    ~ArchivesTabWidget() override;

    //! Expand the "inspect archive" widget.
    void displayInspectArchiveByRef(const QString &archiveName);

public slots:
    //! Expand the "inspect archive" widget.
    void displayInspectArchive(ArchivePtr archive);
    //! Hide the "inspect archive" widget.
    void hideInspectArchive();
    //! Reload the IEC prefix preference and re-display number(s).
    void updateIEC();

signals:
    //! Begin tarsnap --list-archives
    void getArchives();
    //! Passes info from the ArchiveListWidget or JobDetailsWidget to the
    //! TaskManager.
    void deleteArchives(QList<ArchivePtr> archives);
    //! Begin tarsnap -x -f \<name\>, with options.
    void restoreArchive(ArchivePtr archive, ArchiveRestoreOptions options);

    //! Passes the list of all Archive objects to the ArchiveListWidget.
    void archiveList(QList<ArchivePtr> archives);
    //! Passes the creation of a new Archive to the ArchiveListWidget.
    void addArchive(ArchivePtr archive);

    //! Notify that the job details should be displayed.
    void displayJobDetails(const QString &jobRef);
    //! The user clicked on the Job label.
    //! \param jobRef a string which identifies the Job.
    void jobClicked(const QString &jobRef);

    //! Begin tarsnap --print-stats -f \<name\>
    void loadArchiveStats(ArchivePtr archive);
    //! Begin tarsnap --tv -f \<name\>
    void loadArchiveContents(ArchivePtr archive);

    //! We have a task to perform in the background.
    void taskRequested(BaseTask *task);

protected:
    //! Handles translation change of language.
    void changeEvent(QEvent *event) override;
    //! Handles the escape key; passes other events on.
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void showArchiveListMenu();

private:
    Ui::ArchivesTabWidget *_ui;

    QMenu *_archiveListMenu;

    void updateKeyboardShortcutInfo();
};

#endif // ARCHIVESTABWIDGET_H
