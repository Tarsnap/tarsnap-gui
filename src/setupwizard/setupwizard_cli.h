#ifndef SETUPWIZARD_CLI_H
#define SETUPWIZARD_CLI_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QObject>
#include <QString>
WARNINGS_ENABLE

#include "TWizardPage.h"

#include "messages/taskstatus.h"

/* Forward declaration(s). */
namespace Ui
{
class CliPage;
}
class QWidget;
class TPathLineBrowse;

/**
 * \ingroup widgets-setup
 * \brief The CLIPage is a TWizardPage which searches for the
 * CLI and configures directories.
 */
class CliPage : public TWizardPage
{
    Q_OBJECT

public:
    //! Constructor.
    CliPage(QWidget *parent = nullptr);
    ~CliPage() override;

    //! Override the default TWizardPage functions
    void initializePage() override;

#ifdef QT_TESTLIB_LIB
    Ui::CliPage *get_ui() const { return _ui; }
#endif

public slots:
    //! The TaskManager has finished determining the version of the CLI app.
    //! \param status did the task complete?
    //! \param versionString the version number.
    void tarsnapVersionResponse(TaskStatus     status,
                                const QString &versionString);

signals:
    //! We need to determine the Tarsnap CLI version number.
    void tarsnapVersionRequested();

protected:
    //! Has the user filled in all mandatory data on the page?
    bool checkComplete() override;

private slots:
    bool tarsnapPathChanged(const QString &text);
    bool tarsnapCacheChanged(const QString &text);
    bool appDataDirChanged(const QString &text);

private:
    Ui::CliPage *_ui;

    // This will allow us to display a "success" message after
    // temporarily losing it (e.g., after fiddling with dirs).
    QString _successMessage;

    // Has there been any problems?  If there haven't, and the CLI
    // version is high enough, then we want to auto-focus the
    // "Next" button.  But if we've been manually poking around in
    // the details, we don't want to steal the focus from where
    // the user is currently working.
    bool _problemOccurred;

    // We store the final configured state in the TSettings.  Namely:
    // "tarsnap/path": directory of the CLI.
    // "tarsnap/version": version number (if high enough).
    // "tarsnap/cache": directory of tarsnap --cachedir.
    // "app/app_data": directory of the application database and
    //                 default key location.

    bool reportError(const QString &text, TPathLineBrowse *plb = nullptr,
                     const QString &plb_text = "");
    bool ensureDirExists(const QString &dirname);
};

#endif /* !SETUPWIZARD_CLI_H */
