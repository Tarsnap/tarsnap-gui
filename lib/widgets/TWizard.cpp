#include "TWizard.h"

WARNINGS_DISABLE
#include <QDialog>
#include <QLabel>
#include <QStackedWidget>
#include <QWidget>

#include "ui_TWizard.h"
WARNINGS_ENABLE

#include "TWizardPage.h"

TWizard::TWizard(QWidget *parent)
    : QDialog(parent), _ui(new Ui::TWizard), _maxPageInitialized(-1)
{
    _ui->setupUi(this);
}

TWizard::~TWizard()
{
    delete _ui;
}

TWizardPage *TWizard::currentPage() const
{
    QWidget *page = _ui->pageWidget->currentWidget();
    return qobject_cast<TWizardPage *>(page);
}

void TWizard::setLogo(const QPixmap &pixmap)
{
    _ui->logoLabel->setPixmap(pixmap);
}

QString TWizard::pageTitle() const
{
    return _ui->titleLabel->text();
}

void TWizard::addPages(QList<TWizardPage *> pages)
{
    for(TWizardPage *page : pages)
        addPage(page);
    setupCurrentPage();
}

void TWizard::addPage(TWizardPage *page)
{
    _ui->pageWidget->addWidget(page);

    // Navigation
    connect(page, &TWizardPage::skipWizard, this, &TWizard::skipWizard);
    connect(page, &TWizardPage::backPage, this, &TWizard::backPage);
    connect(page, &TWizardPage::nextPage, this, &TWizard::nextPage);
    connect(page, &TWizardPage::finishWizard, this, &TWizard::finishWizard);
}

void TWizard::skipWizard()
{
    accept();
}

void TWizard::backPage()
{
    const int pos = _ui->pageWidget->currentIndex();
    Q_ASSERT(pos > 0);
    _ui->pageWidget->setCurrentIndex(pos - 1);
    setupCurrentPage();
}

void TWizard::nextPage()
{
    const int pos = _ui->pageWidget->currentIndex();
    Q_ASSERT(pos < _ui->pageWidget->count() - 1);
    _ui->pageWidget->setCurrentIndex(pos + 1);
    setupCurrentPage();
}

void TWizard::finishWizard()
{
    accept();
}

void TWizard::setupCurrentPage()
{
    TWizardPage *page = currentPage();
    Q_ASSERT(page != nullptr);

    // Set title
    _ui->titleLabel->setText(page->title());

    // Initialize the page (if required)
    const int pos = _ui->pageWidget->currentIndex();
    if(pos > _maxPageInitialized)
    {
        page->initializePage();
        _maxPageInitialized = pos;
    }
}
