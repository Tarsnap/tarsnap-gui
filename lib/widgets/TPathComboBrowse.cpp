#include "TPathComboBrowse.h"

WARNINGS_DISABLE
#include <QComboBox>
#include <QEvent>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include "ui_TPathComboBrowse.h"
WARNINGS_ENABLE

#include "TOkLabel.h"

TPathComboBrowse::TPathComboBrowse(QWidget *parent)
    : QWidget(parent), _ui(new Ui::TPathComboBrowse)
{
    _ui->setupUi(this);

    // We want to allow the combined widget to accept the focus (in
    // the UI), but that focus should be redirected to the comboBox.
    setFocusProxy(_ui->comboBox);

    // External connections
    connect(_ui->comboBox, &QComboBox::currentTextChanged, this,
            &TPathComboBrowse::textChanged);

    // Internal connections
    connect(_ui->button, &QPushButton::clicked, this,
            &TPathComboBrowse::browseClicked);
}

TPathComboBrowse::~TPathComboBrowse()
{
    delete _ui;
}

void TPathComboBrowse::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
        _ui->retranslateUi(this);
    else
        QWidget::changeEvent(event);
}

QString TPathComboBrowse::label() const
{
    return _ui->label->text();
}

void TPathComboBrowse::setLabel(const QString &text)
{
    _ui->label->setText(text);
}

QString TPathComboBrowse::placeholderText() const
{
    return _ui->comboBox->lineEdit()->placeholderText();
}

void TPathComboBrowse::setPlaceholderText(const QString &text)
{
    _ui->comboBox->lineEdit()->setPlaceholderText(text);
}

QString TPathComboBrowse::dialogTitle() const
{
    return _dialogTitle;
}

void TPathComboBrowse::setDialogTitle(const QString &text)
{
    _dialogTitle = text;
}

QString TPathComboBrowse::dialogFilter() const
{
    return _dialogFilter;
}

void TPathComboBrowse::setDialogFilter(const QString &text)
{
    _dialogFilter = text;
}

QString TPathComboBrowse::statusText() const
{
    return _ui->statusLabel->text();
}

void TPathComboBrowse::setStatusOk(const QString &text)
{
    _ui->okLabel->setStatus(TOkLabel::Ok);
    _ui->statusLabel->setText(text);
    _ui->statusLabel->setStyleSheet("");
    _ui->comboBox->lineEdit()->setStyleSheet("");
}

void TPathComboBrowse::setStatusError(const QString &text)
{
    _ui->okLabel->setStatus(TOkLabel::Error);
    _ui->statusLabel->setText(text);
    _ui->statusLabel->setStyleSheet("color: darkred;");
    _ui->comboBox->lineEdit()->setStyleSheet("color: darkred;");
}

QString TPathComboBrowse::text() const
{
    return _ui->comboBox->currentText();
}

void TPathComboBrowse::setText(const QString &text)
{
    // If the text is already present, set the index to it.
    for(int i = 0; i < count(); i++)
    {
        if(_ui->comboBox->itemText(i) == text)
        {
            _ui->comboBox->setCurrentIndex(i);
            return;
        }
    }

    // Otherwise, add it.
    _ui->comboBox->addItem(text);
    _ui->comboBox->setCurrentText(text);
}

void TPathComboBrowse::clear()
{
    _ui->comboBox->clear();
    _ui->comboBox->clearEditText();
    // Clear status-related items.
    _ui->okLabel->setStatus(TOkLabel::Unset);
    _ui->statusLabel->setText("");
    _ui->statusLabel->setStyleSheet("");
    _ui->comboBox->lineEdit()->setStyleSheet("");
}

int TPathComboBrowse::count() const
{
    return _ui->comboBox->count();
}

void TPathComboBrowse::addItem(const QString &text)
{
    _ui->comboBox->addItem(text);
}

void TPathComboBrowse::browseClicked()
{
    // Run the file browser
    QString dialogResult =
        QFileDialog::getOpenFileName(this, _dialogTitle,
                                     _ui->comboBox->currentText(),
                                     tr("All files (*);;") + _dialogFilter,
                                     &_dialogFilter);

    // Handle result
    if(dialogResult.isEmpty())
        return;
    _ui->comboBox->setCurrentText(dialogResult);
}
