#ifndef SETUPWIZARD_INTRO_H
#define SETUPWIZARD_INTRO_H

#include <TWizardPage.h>

/* Forward declaration(s). */
namespace Ui
{
class IntroPage;
}

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
    ~IntroPage();

private:
    Ui::IntroPage *_ui;
};

#endif /* !SETUPWIZARD_INTRO_H */
