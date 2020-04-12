#include "TPathLineBrowse.h"

WARNINGS_DISABLE
#include <QEvent>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include "ui_TPathLineBrowse.h"
WARNINGS_ENABLE

#include "TOkLabel.h"

TPathLineBrowse::TPathLineBrowse(QWidget *parent)
    : QWidget(parent), _ui(new Ui::TPathLineBrowse)
{
    _ui->setupUi(this);

    // We want to allow the combined widget to accept the focus (in
    // the UI), but that focus should be redirected to the lineEdit.
    setFocusProxy(_ui->lineEdit);

    // External connections
    connect(_ui->lineEdit, &QLineEdit::textChanged, this,
            &TPathLineBrowse::textChanged);

    // Internal connections
    connect(_ui->button, &QPushButton::clicked, this,
            &TPathLineBrowse::browseClicked);
}

TPathLineBrowse::~TPathLineBrowse()
{
    delete _ui;
}

void TPathLineBrowse::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
        _ui->retranslateUi(this);
    else
        QWidget::changeEvent(event);
}

QString TPathLineBrowse::label() const
{
    return _ui->label->text();
}

void TPathLineBrowse::setLabel(const QString &text)
{
    _ui->label->setText(text);
}

QString TPathLineBrowse::placeholderText() const
{
    return _ui->lineEdit->placeholderText();
}

void TPathLineBrowse::setPlaceholderText(const QString &text)
{
    _ui->lineEdit->setPlaceholderText(text);
}

QString TPathLineBrowse::dialogTitle() const
{
    return _dialogTitle;
}

void TPathLineBrowse::setDialogTitle(const QString &text)
{
    _dialogTitle = text;
}

QString TPathLineBrowse::text() const
{
    return _ui->lineEdit->text();
}

void TPathLineBrowse::setText(const QString &text)
{
    _ui->lineEdit->setText(text);
}

void TPathLineBrowse::clear()
{
    _ui->lineEdit->clear();
    // Clear status-related items.
    _ui->okLabel->setStatus(TOkLabel::Unset);
    _ui->statusLabel->setText("");
    _ui->statusLabel->setStyleSheet("");
    _ui->lineEdit->setStyleSheet("");
}

QString TPathLineBrowse::statusText() const
{
    return _ui->statusLabel->text();
}

void TPathLineBrowse::setStatusOk(const QString &text)
{
    _ui->okLabel->setStatus(TOkLabel::Ok);
    _ui->statusLabel->setText(text);
    _ui->statusLabel->setStyleSheet("");
    _ui->lineEdit->setStyleSheet("");
}

void TPathLineBrowse::setStatusError(const QString &text)
{
    _ui->okLabel->setStatus(TOkLabel::Error);
    _ui->statusLabel->setText(text);
    _ui->statusLabel->setStyleSheet("color: darkred;");
    _ui->lineEdit->setStyleSheet("color: darkred");
}

void TPathLineBrowse::browseClicked()
{
    // Run the file browser
    QString dialogResult =
        QFileDialog::getExistingDirectory(this, _dialogTitle,
                                          _ui->lineEdit->text());

    // Handle result
    if(dialogResult.isEmpty())
        return;
    _ui->lineEdit->setText(dialogResult);
}
