#ifndef ARCHIVELISTWIDGET_H
#define ARCHIVELISTWIDGET_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QList>
#include <QListWidget>
#include <QObject>
#include <QString>
WARNINGS_ENABLE

#include "messages/archiveptr.h"
#include "messages/archiverestoreoptions.h"

/* Forward declaration(s). */
class ArchiveListWidgetItem;
class QWidget;

/*!
 * \ingroup widgets-specialized
 * \brief The ArchiveListWidget is a QListWidget which displays
 * information about all archives.
 */
class ArchiveListWidget : public QListWidget
{
    Q_OBJECT

public:
    //! Constructor.
    explicit ArchiveListWidget(QWidget *parent = nullptr);
    ~ArchiveListWidget() override;

    //! Make sure that we can see the current item in the list.
    void ensureCurrentItemVisible();

    //! Find the archive matching the given name.
    ArchivePtr findArchiveByName(const QString &archiveName);

    //! Reload the IEC prefix preference and re-display number(s).
    void updateIEC();

public slots:
    //! Clears the archive list, then sets it to the specified archives
    void setArchives(QList<ArchivePtr> archives);
    //! Adds an archive to the list.
    void addArchive(ArchivePtr archive);
    //! Sets the current selection in the list view.
    void selectArchive(ArchivePtr archive);
    //! Delete the selected archives.
    void deleteSelectedItems();
    //! Displays detailed information about the first of the selected items.
    void inspectSelectedItem();
    //! Restore the first of the selected archives.
    void restoreSelectedItem();
    //! Filter the list of archives.
    void setFilter(const QString &regex);
    //! There are no archive details being shown.
    void noInspect();

signals:
    //! Notify that the specified archives should be deleted from
    //! the Tarsnap servers.
    void deleteArchives(QList<ArchivePtr> archives);
    //! Notify that detailed information should be shown about the
    //! specified archive.
    void inspectArchive(ArchivePtr archive);
    //! Notify that no detailed information should be shown.
    void clearInspectArchive();
    //! Notify that the specified archive should be restored,
    //! using the user-selected options from the \ref RestoreDialog.
    void restoreArchive(ArchivePtr archive, ArchiveRestoreOptions options);
    //! Notify that the job details should be displayed.
    void displayJobDetails(const QString &jobRef);
    //! Notify the total and visible (not hidden) items count on list change
    //! (item added, removed or hidden).
    void countChanged(int countTotal, int countVisible);

protected:
    //! Handles the delete and escape keys; passes other events on.
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void deleteItem();
    void inspectItem();
    void restoreItem();
    void goToJob();
    void removeItem();
    void handleItemActivated(QListWidgetItem *item);

private:
    int  visibleItemsCount();
    void insertArchive(ArchivePtr archive, int pos);

    QRegExp *_filter;

    void goingToInspectItem(ArchiveListWidgetItem *archiveItem);
    ArchiveListWidgetItem *_highlightedItem;
};

#endif // ARCHIVELISTWIDGET_H
