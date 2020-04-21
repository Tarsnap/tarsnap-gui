#include "setupwizard/setupwizard_intro.h"

#include "warnings-disable.h"

WARNINGS_DISABLE
#include "ui_setupwizard_intro.h"
WARNINGS_ENABLE

/* Forward declaration(s). */
class QWidget;

#include "TWizardPage.h"

IntroPage::IntroPage(QWidget *parent)
    : TWizardPage(parent), _ui(new Ui::IntroPage)
{
    _ui->setupUi(this);
}

IntroPage::~IntroPage()
{
    delete _ui;
}
