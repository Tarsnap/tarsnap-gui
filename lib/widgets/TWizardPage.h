#ifndef TWIZARDPAGE_H
#define TWIZARDPAGE_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QObject>
#include <QString>
#include <QWidget>
WARNINGS_ENABLE

/* Forward declaration(s). */
class QAbstractButton;

/*!
 * \ingroup widgets
 * \brief The TWizardPage is a QWidget which is similar to a QWizardPage,
 * but with more control over the appearance and behaviour.
 *
 * The major divergence from QWizardPage is that the navigation buttons are
 * within the UI for each page, not the main TWizard dialog.  This allows
 * TWizardPage to decide whether or not to enable the Next or Finish button,
 * what to do if the user clicks on it, etc.  All that the TWizard care
 * about is whether it receives a NextPage or FinishPage signal.
 *
 * This avoids the whole QWizardPage::isComplete(),
 * QWizardPage::validatePage(), and QWizardPage::completeChanged()
 * infrastructure.  In addition, it makes keyboard navigation (and focus)
 * easier for Qt to handle, thereby avoiding an odd bug wherein a navigation
 * button and widget inside a QWizardPage both appearing to have keyboard
 * focus at the same time.
 */
class TWizardPage : public QWidget
{
    Q_OBJECT

    //! Page title.
    Q_PROPERTY(QString title READ title WRITE setTitle DESIGNABLE true)

public:
    //! Constructor.
    explicit TWizardPage(QWidget *parent = nullptr);

    //! Initialize the page when it first becomes visible.
    virtual void initializePage();

    //! Get the current page's title.
    QString title() const;
    //! Set the title.
    void setTitle(const QString &text);

    //! Navigation buttons.
    enum WizardButton
    {
        SkipButton,
        BackButton,
        NextButton,
        FinishButton
    };

    //! Get a navigation button.
    QAbstractButton *button(const WizardButton which) const;

signals:
    //! Stop the wizard at the current point and accept the current config.
    void skipWizard();
    //! Move to the previous page.
    void backPage();
    //! Move to the next page.
    void nextPage();
    //! Finish the wizard.
    void finishWizard();

protected:
    //! User activated the "skip" button.
    virtual void skip();
    //! User activated the "back" button.
    virtual void back();
    //! User activated the "next" button.
    virtual void next();
    //! User activated the "finish" button.
    virtual void finish();

    //! Has the user filled in all mandatory data on the page?
    virtual bool checkComplete();

    //! Enable (or not) the Next or Finish button.
    bool setProceedButton(bool enable);

private:
    QString _title;

    //! Either the "next" or "finish" button.
    QAbstractButton *_proceedButton;

    //! Focus the proceed button (for easy keyboard navigation).
    void focus_proceed_button();

    // Called in initializePage, after the UI has been set up.
    void connect_ui_buttons();
};

#endif /* TWIZARDPAGE_H */
