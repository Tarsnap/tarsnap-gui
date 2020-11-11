#ifndef SETUPWIZARD_INTRO_H
#define SETUPWIZARD_INTRO_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QObject>
WARNINGS_ENABLE

#include "TWizardPage.h"

/* Forward declaration(s). */
namespace Ui
{
class IntroPage;
}
class QWidget;

/**
 * \ingroup widgets-setup
 * \brief The IntroPage is a TWizardPage which introduces the user
 * to the SetupWizard.
 */
class IntroPage : public TWizardPage
{
    Q_OBJECT

public:
    //! Constructor.
    IntroPage(QWidget *parent = nullptr);
    ~IntroPage() override;

private:
    Ui::IntroPage *_ui;
};

#endif /* !SETUPWIZARD_INTRO_H */
