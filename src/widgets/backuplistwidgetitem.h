#ifndef BACKUPLISTWIDGETITEM_H
#define BACKUPLISTWIDGETITEM_H

#include "ui_backuplistwidgetitem.h"

#include <QListWidgetItem>
#include <QObject>
#include <QUrl>

/*!
 * \ingroup widgets-specialized
 * \brief The BackupListWidgetItem is a QListWidgetItem which
 * displays information about a file or directory to be backed up.
 */
class BackupListWidgetItem : public QObject, public QListWidgetItem
{
    Q_OBJECT

public:
    //! Constructor.
    //! \param url: The url of the file or directory to include in
    //!             this archive.
    explicit BackupListWidgetItem(QUrl url);
    ~BackupListWidgetItem();

    //! Returns a widget with a graphical representation of this
    //! file or directory.
    QWidget *widget();

    //! Returns the url of this file or directory.
    QUrl url() const;

    //! Returns the number of files in this file or directory.
    quint64 count() const;

    //! Returns the total size of this file or directory.
    quint64 size() const;

public slots:
    //! Displays the file or directory using the default
    //! desktop application.
    void browseUrl();

signals:
    //! The user requested to remove this item from the backup list.
    void requestDelete();
    //! The count and size of this directory have been (re)calculated.
    void requestUpdate();

protected:
    //! Processes requests to switch language.
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void updateDirDetail(quint64 size, quint64 count);

private:
    Ui::BackupListWidgetItem _ui;
    QWidget             *_widget;
    QUrl                 _url;
    quint64              _count;
    quint64              _size;

    void setUrl(const QUrl &url);
};

#endif // BACKUPLISTITEM_H
