#include "app-setup.h"

WARNINGS_DISABLE
#include <QDialog>
#include <QMessageBox>
#include <QTimer>
#include <QVariant>
WARNINGS_ENABLE

#include "TSettings.h"

#include "messages/taskstatus.h"

#include "debug.h"
#include "init-shared.h"
#include "setupwizard/setupwizard.h"
#include "taskmanager.h"
#include "translator.h"

AppSetup::AppSetup(int &argc, char **argv)
    : QApplication(argc, argv), _wizard(nullptr), _taskManager(nullptr)
{
    // Set up the translator.
    TSettings settings;
    Translator::initializeTranslator();
    global_translator->translateApp(
        this, settings.value("app/language", LANG_AUTO).toString());
}

AppSetup::~AppSetup()
{
    if(_wizard != nullptr)
        delete _wizard;
    if(_taskManager != nullptr)
        delete _taskManager;
    Translator::destroy();
}

bool AppSetup::handle_init(const QList<struct init_info> &steps)
{
    // Handle each step of the initialization
    for(const struct init_info &info : steps)
    {
        if(!handle_step(info))
            return false;
    }

    // We've finished initialization and can proceed to prepMainLoop().
    return true;
}

/*
 * Show message(s) (if applicable), and return false if there's an error.
 *
 * In order for Qt's translation tr() to work, this must be a class method
 * (rather than a static function).
 */
bool AppSetup::handle_step(const struct init_info &info)
{
    switch(info.status)
    {
    case INIT_OK:
        return true;
    case INIT_NEEDS_SETUP:
        return true;
    case INIT_DB_FAILED:
        QMessageBox::warning(nullptr, tr("Tarsnap warning"),
                             tr("Cannot initialize the database."));
        return false;
    case INIT_SETTINGS_RENAMED:
        QMessageBox::information(nullptr, tr("Tarsnap info"), info.message);
        return true;
    case INIT_DRY_RUN:
        QMessageBox::warning(nullptr, tr("Tarsnap warning"), info.message);
        break;
    case INIT_SCHEDULE_OK:
        QMessageBox::information(nullptr, tr("Updated OS X launchd path"),
                                 info.message);
        return true;
    case INIT_SCHEDULE_ERROR:
        QMessageBox::information(nullptr,
                                 tr("Failed to updated OS X launchd path"),
                                 info.message);
        return true;
    }

    // Should not happen
    DEBUG << "AppSetup: unexpected info.status:" << info.status;
    return false;
}

bool AppSetup::prepEventLoop()
{
    // Create the objects.
    _taskManager = new TaskManager();
    _wizard      = new SetupWizard();

    // Connect the frontend <-> backend signals.
    connect(_wizard, &SetupWizard::tarsnapVersionRequested, _taskManager,
            &TaskManager::tarsnapVersionFind);
    connect(_taskManager, &TaskManager::tarsnapVersionFound, _wizard,
            &SetupWizard::tarsnapVersionResponse);
    connect(_wizard, &SetupWizard::registerMachineRequested, _taskManager,
            &TaskManager::registerMachineDo);
    connect(_taskManager, &TaskManager::registerMachineProgress, _wizard,
            &SetupWizard::registerMachineProgress);
    connect(_taskManager, &TaskManager::registerMachineDone, _wizard,
            &SetupWizard::registerMachineResponse);

    // Connect the backend -> frontend signal.
    connect(_taskManager, &TaskManager::numTasks, _wizard,
            &SetupWizard::updateNumTasks);

    // Connect the frontend -> this signal.
    connect(_wizard, &SetupWizard::finished, this, &AppSetup::finished);

    // Launch the wizard (non-blocking).
    _wizard->open();

#ifdef QT_TESTLIB_LIB
    QTimer::singleShot(0, this, SLOT(onEventLoopStarted()));
#endif

    // Ready for the event loop.
    return true;
}

void AppSetup::finished(int result)
{
    if(result == QDialog::Accepted)
        QApplication::exit(0);
    else
        QApplication::exit(1);
}

#ifdef QT_TESTLIB_LIB
void AppSetup::fakeNextTask()
{
    Q_ASSERT(_taskManager != nullptr);
    _taskManager->fakeNextTask();
}

void AppSetup::waitUntilIdle()
{
    Q_ASSERT(_taskManager != nullptr);
    _taskManager->waitUntilIdle();
}

void AppSetup::onEventLoopStarted()
{
    Q_ASSERT(_wizard != nullptr);
    emit eventLoopStarted(this);
}
#endif
