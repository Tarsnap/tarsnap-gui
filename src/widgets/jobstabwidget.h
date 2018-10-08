#ifndef JOBSTABWIDGET_H
#define JOBSTABWIDGET_H

#include "ui_jobstabwidget.h"

#include "persistentmodel/job.h"

#include <QEvent>
#include <QKeyEvent>
#include <QWidget>

/*!
 * \ingroup widgets-main
 * \brief The JobsTabWidget is a QWidget which shows the list of jobs.
 */
class JobsTabWidget : public QWidget
{
    Q_OBJECT

#ifdef QT_TESTLIB_LIB
    friend class TestJobsTabWidget;
#endif

public:
    //! Constructor.
    explicit JobsTabWidget(QWidget *parent = nullptr);

public slots:

signals:
    //! Temporary to ease the JobsTabWidget refactoring
    void temp_addJobButton_show(bool show);
    //! Temporary to ease the JobsTabWidget refactoring
    void temp_jobDetailsWidget_jobAdded(JobPtr job);

protected:
    //! Handles translation change of language.
    void changeEvent(QEvent *event);
    //! Handles the escape key; passes other events on.
    void keyPressEvent(QKeyEvent *event);

private slots:
    void addDefaultJobs();

private:
    Ui::JobsTabWidget _ui;

    void updateUi();
    void loadSettings();
};

#endif // JOBSTABWIDGET_H
