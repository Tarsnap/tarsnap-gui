#include "TWizardPage.h"

WARNINGS_DISABLE
#include <QAbstractButton>
#include <QString>
#include <QWidget>
WARNINGS_ENABLE

#define GET_BUTTON(BUTTON) button = findChild<QAbstractButton *>(BUTTON)

TWizardPage::TWizardPage(QWidget *parent)
    : QWidget(parent), _proceedButton(nullptr)
{
}

void TWizardPage::initializePage()
{
    connect_ui_buttons();
    checkComplete();
    focus_proceed_button();
}

bool TWizardPage::checkComplete()
{
    return setProceedButton(true);
}

bool TWizardPage::setProceedButton(bool enable)
{
    _proceedButton->setEnabled(enable);
    return enable;
}

QString TWizardPage::title() const
{
    return _title;
}

void TWizardPage::setTitle(const QString &text)
{
    _title = text;
}

void TWizardPage::connect_ui_buttons()
{
    QAbstractButton *button;

    if((GET_BUTTON("skipButton")) != nullptr)
        connect(button, &QAbstractButton::clicked, this, &TWizardPage::skip);
    if((GET_BUTTON("backButton")) != nullptr)
        connect(button, &QAbstractButton::clicked, this, &TWizardPage::back);
    if((GET_BUTTON("nextButton")) != nullptr)
    {
        connect(button, &QAbstractButton::clicked, this, &TWizardPage::next);
        Q_ASSERT(_proceedButton == nullptr);
        _proceedButton = button;
    }
    if((GET_BUTTON("finishButton")) != nullptr)
    {
        connect(button, &QAbstractButton::clicked, this, &TWizardPage::finish);
        Q_ASSERT(_proceedButton == nullptr);
        _proceedButton = button;
    }
    Q_ASSERT(_proceedButton != nullptr);
}

QAbstractButton *TWizardPage::button(const WizardButton which) const
{
    QAbstractButton *button;
    switch(which)
    {
    case SkipButton:
        GET_BUTTON("skipButton");
        Q_ASSERT(button != nullptr);
        return button;
    case BackButton:
        GET_BUTTON("backButton");
        Q_ASSERT(button != nullptr);
        return button;
    case NextButton:
        GET_BUTTON("nextButton");
        Q_ASSERT(button != nullptr);
        return button;
    case FinishButton:
        GET_BUTTON("finishButton");
        Q_ASSERT(button != nullptr);
        return button;
    }
    // Shouldn't reach here.
    return nullptr;
}

void TWizardPage::focus_proceed_button()
{
    QAbstractButton *button;

    if((GET_BUTTON("nextButton")) != nullptr)
        button->setFocus();
    else if((GET_BUTTON("finishButton")) != nullptr)
        button->setFocus();
}

// These methods are here so that derived classes can override them.
void TWizardPage::skip()
{
    emit skipWizard();
}

void TWizardPage::back()
{
    emit backPage();
}

void TWizardPage::next()
{
    emit nextPage();
}

void TWizardPage::finish()
{
    emit finishWizard();
}
