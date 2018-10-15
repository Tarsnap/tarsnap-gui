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

    //! Does the Backup have a name?
    bool validateBackupTab();

    // Temp for refactoring
    QLineEdit *temp_backupNameLineEdit() { return _ui.backupNameLineEdit; }
    QCheckBox *temp_appendTimestampCheckBox()
    {
        return _ui.appendTimestampCheckBox;
    }
    QLabel *     temp_backupDetailLabel() { return _ui.backupDetailLabel; }
    QToolButton *temp_backupButton() { return _ui.backupButton; }

protected:
    //! Handles translation change of language.
    void changeEvent(QEvent *event);

private slots:
    void appendTimestampCheckBoxToggled(bool checked);

private:
    Ui::BackupTabWidget _ui;

    QString _lastTimestamp;

    void updateUi();
};

#endif // BACKUPTABWIDGET_H
