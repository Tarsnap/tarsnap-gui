#ifndef ARCHIVELISTWIDGETITEM_H
#define ARCHIVELISTWIDGETITEM_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QListWidgetItem>
#include <QObject>
WARNINGS_ENABLE

#include "messages/archiveptr.h"

/* Forward declaration(s). */
namespace Ui
{
class ArchiveListWidgetItem;
}
class QEvent;
class QWidget;

/*!
 * \ingroup widgets-specialized
 * \brief The ArchiveListWidgetItem is a QListWidgetItem which
 * displays information about an archive.
 */
class ArchiveListWidgetItem : public QObject, public QListWidgetItem
{
    Q_OBJECT

public:
    //! Constructor.
    //! \param archive pointer to the archive to display.
    explicit ArchiveListWidgetItem(ArchivePtr archive);
    ~ArchiveListWidgetItem() override;

    //! Returns a widget with a graphical representation of this archive.
    QWidget *widget();

    //! Returns a pointer to the archive.
    ArchivePtr archive() const;

    //! Set whether this item's details are being shown in the
    //! ArchiveDetailsWidget.
    void setShowingDetails(bool is_showing);

    //! Reload the IEC prefix preference and re-display number(s).
    void updateIEC();

signals:
    //! The user requested to delete this archive.
    void requestDelete();
    //! The user requested more information about this archive.
    void requestInspect();
    //! The user requested to restore this archive.
    void requestRestore();
    //! The user requested to see the Job which created this archive.
    void requestGoToJob();
    //! Remove this item from the Archive list.
    void removeItem();

protected:
    //! Processes requests to switch language.
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    // Modify the display if the Archive is scheduled for deletion.
    void updateStatus();

private:
    Ui::ArchiveListWidgetItem *_ui;
    QWidget                   *_widget;
    ArchivePtr                 _archive;

    void setArchive(ArchivePtr archive);
    void updateKeyboardShortcutInfo();
};

#endif // ARCHIVELISTWIDGETITEM_H
