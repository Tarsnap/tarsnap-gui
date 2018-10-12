#ifndef BACKUPTABWIDGET_H
#define BACKUPTABWIDGET_H

#include "ui_backuptabwidget.h"

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
#endif

public:
    //! Constructor.
    explicit BackupTabWidget(QWidget *parent = nullptr);

protected:
    //! Handles translation change of language.
    void changeEvent(QEvent *event);

private slots:

private:
    Ui::BackupTabWidget _ui;

    void updateUi();
};

#endif // BACKUPTABWIDGET_H
