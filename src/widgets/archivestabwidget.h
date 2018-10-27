#ifndef ARCHIVESTABWIDGET_H
#define ARCHIVESTABWIDGET_H

#include "ui_archivestabwidget.h"

#include <QEvent>
#include <QKeyEvent>
#include <QWidget>

/*!
 * \ingroup widgets-main
 * \brief The ArchivesTabWidget is a QWidget which shows the Archives tab.
 */
class ArchivesTabWidget : public QWidget
{
    Q_OBJECT

public:
    //! Constructor.
    explicit ArchivesTabWidget(QWidget *parent = nullptr);

public slots:
    //! Expand the "inspect archive" widget.
    void displayInspectArchive(ArchivePtr archive);

signals:
    //! Passes info from the ArchiveListWidget or JobWidget to the TaskManager.
    void deleteArchives(QList<ArchivePtr> archives);
    //! Begin tarsnap -x -f \<name\>, with options.
    void restoreArchive(ArchivePtr archive, ArchiveRestoreOptions options);

    //! Passes the list of all Archive objects to the ArchiveListWidget.
    void archiveList(QList<ArchivePtr> archives);
    //! Passes the creation of a new Archive to the ArchiveListWidget.
    void addArchive(ArchivePtr archive);

    //! Notify that the job details should be displayed.
    void displayJobDetails(QString jobRef);
    //! The user clicked on the Job label.
    //! \param jobRef: a string which identifies the Job.
    void jobClicked(QString jobRef);

    //! Begin tarsnap --print-stats -f \<name\>
    void loadArchiveStats(ArchivePtr archive);
    //! Begin tarsnap --tv -f \<name\>
    void loadArchiveContents(ArchivePtr archive);

protected:
    //! Handles translation change of language.
    void changeEvent(QEvent *event);
    //! Handles the escape key; passes other events on.
    void keyPressEvent(QKeyEvent *event);

private slots:
    void showArchiveListMenu(const QPoint &pos);

private:
    Ui::ArchivesTabWidget _ui;

    void updateUi();
};

#endif // ARCHIVESTABWIDGET_H
