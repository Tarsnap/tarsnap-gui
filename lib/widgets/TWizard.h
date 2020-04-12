#ifndef TWIZARD_H
#define TWIZARD_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QDialog>
#include <QList>
#include <QObject>
#include <QString>
WARNINGS_ENABLE

/* Forward declaration(s). */
namespace Ui
{
// Using a UI form for this allows us to avoid a fair chunk of
// boilerplate code in creating the layouts and labels.
class TWizard;
} // namespace Ui
class QPixmap;
class QWidget;
class TWizardPage;

/*!
 * \ingroup lib-widgets
 * \brief The TWizard is a QDialog which is similar to a QWizard,
 * but with more control over the appearance.
 *
 * TWizard diverges from QWizard in a few important aspects:
 * - it still uses TWizardPages, but it is not a subclass of QWizard.
 * - pages must be added all at once via TWizard::addPages(), instead of
 *   one-by-one with QWizard::addPage().
 * - it does not support the field() and registerField() mechanism.
 * - it only supports a linear wizard, rather than non-linear ones.
 * - it does not support subtitles.
 * - each page can have up to four buttons: Skip, Back, Next, Finish.
 *   Each page must have at at least one "proceed" button (Next or Finish).
 */
class TWizard : public QDialog
{
    Q_OBJECT

public:
    //! Constructor.
    explicit TWizard(QWidget *parent = nullptr);
    ~TWizard() override;

    //! Get the current page.
    TWizardPage *currentPage() const;

    //! Set the 32x32-pixel logo in the upper-right corner.
    void setLogo(const QPixmap &pixmap);

    //! Get the current page title.
    QString pageTitle() const;

    //! Add pages to the wizard.
    void addPages(QList<TWizardPage *> pages);

protected slots:
    //! Quit the wizard with a QDialog::Accept signal.  (Probably
    //! without visiting every page.)
    virtual void skipWizard();
    //! Move to the previous page.
    void backPage();
    //! Move to the next page.
    void nextPage();
    //! Quit the wizard with a QDialog::Accept signal.
    virtual void finishWizard();

private:
    Ui::TWizard *_ui;

    // Currently, we assume that the wizard is linear.  To support
    // non-linear wizards, this would need to be a list.
    int _maxPageInitialized;

    // Add a TWizardPage to the wizard.
    void addPage(TWizardPage *page);

    // Set up the current page.
    void setupCurrentPage();
};

#endif /* !TWIZARD_H */
