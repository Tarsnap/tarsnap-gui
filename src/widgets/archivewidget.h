#ifndef ARCHIVEWIDGET_H
#define ARCHIVEWIDGET_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QMenu>
#include <QObject>
#include <QString>
#include <QWidget>
WARNINGS_ENABLE

#include "messages/archiveptr.h"
#include "messages/archiverestoreoptions.h"

#include "filetablemodel.h"

/* Forward declaration(s). */
namespace Ui
{
class ArchiveWidget;
}
class BaseTask;
class QCloseEvent;
class QEvent;
class QKeyEvent;
class QPoint;
class QSortFilterProxyModel;

/*!
 * \ingroup widgets-specialized
 * \brief The ArchiveWidget is a QWidget which displays detailed
 * information about a single Archive.
 */
class ArchiveWidget : public QWidget
{
    Q_OBJECT

#ifdef QT_TESTLIB_LIB
    friend class TestArchivesTabWidget;
    friend class TestMainWindow;
#endif

public:
    //! Constructor
    explicit ArchiveWidget(QWidget *parent = nullptr);
    ~ArchiveWidget();

public slots:
    //! Sets the Archive whose details this widget should display.
    void setArchive(ArchivePtr archive);

signals:
    //! The user clicked on the Job label.
    //! \param jobRef a string which identifies the Job.
    void jobClicked(QString jobRef);
    //! The user wants to restore some or all of the files in this Archive.
    //! \param archive the Archive this widget is displaying.
    //! \param options contains the list of files to restore.
    void restoreArchive(ArchivePtr archive, ArchiveRestoreOptions options);
    //! We are no longer showing the details widget.
    void hidden();
    //! We have a task to perform in the background.
    void taskRequested(BaseTask *task);

protected:
    //! This widget is closing; release memory.
    void closeEvent(QCloseEvent *event);
    //! Allow ESC to close the filename filter box.
    void keyPressEvent(QKeyEvent *event);
    //! Handles translation change of language.
    void changeEvent(QEvent *event);

private slots:
    void showContextMenu(const QPoint &pos);
    void restoreFiles();
    void updateDetails();

private:
    Ui::ArchiveWidget *    _ui;
    ArchivePtr             _archive;
    FileTableModel         _contentsModel;
    QSortFilterProxyModel *_proxyModel;
    QMenu                  _fileMenu;

    void updateUi();
};

#endif // ARCHIVEWIDGET_H
