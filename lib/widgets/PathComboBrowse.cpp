#include "PathComboBrowse.h"

#include <QFileDialog>
#include <QLineEdit>

#include "ui_PathComboBrowse.h"

PathComboBrowse::PathComboBrowse(QWidget *parent)
    : QWidget(parent), _ui(new Ui::PathComboBrowse)
{
    _ui->setupUi(this);

    // We want to allow the combined widget to accept the focus (in
    // the UI), but that focus should be redirected to the comboBox.
    setFocusProxy(_ui->comboBox);

    // External connections
    connect(_ui->comboBox, &QComboBox::currentTextChanged, this,
            &PathComboBrowse::textChanged);

    // Internal connections
    connect(_ui->button, &QPushButton::clicked, this,
            &PathComboBrowse::browseClicked);
}

PathComboBrowse::~PathComboBrowse()
{
    delete _ui;
}

QString PathComboBrowse::label() const
{
    return _ui->label->text();
}

void PathComboBrowse::setLabel(const QString &text)
{
    _ui->label->setText(text);
}

QString PathComboBrowse::placeholderText() const
{
    return _ui->comboBox->lineEdit()->placeholderText();
}

void PathComboBrowse::setPlaceholderText(const QString &text)
{
    _ui->comboBox->lineEdit()->setPlaceholderText(text);
}

QString PathComboBrowse::dialogTitle() const
{
    return _dialogTitle;
}

void PathComboBrowse::setDialogTitle(const QString &text)
{
    _dialogTitle = text;
}

QString PathComboBrowse::dialogFilter() const
{
    return _dialogFilter;
}

void PathComboBrowse::setDialogFilter(const QString &text)
{
    _dialogFilter = text;
}

QString PathComboBrowse::statusText() const
{
    return _ui->statusLabel->text();
}

void PathComboBrowse::setStatusOk(const QString &text)
{
    _ui->okLabel->setStatus(OkLabel::Ok);
    _ui->statusLabel->setText(text);
    _ui->statusLabel->setStyleSheet("");
    _ui->comboBox->lineEdit()->setStyleSheet("");
}

void PathComboBrowse::setStatusError(const QString &text)
{
    _ui->okLabel->setStatus(OkLabel::Error);
    _ui->statusLabel->setText(text);
    _ui->statusLabel->setStyleSheet("color: darkred;");
    _ui->comboBox->lineEdit()->setStyleSheet("color: darkred;");
}

QString PathComboBrowse::text() const
{
    return _ui->comboBox->currentText();
}

void PathComboBrowse::setText(const QString &text)
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

void PathComboBrowse::clear()
{
    _ui->comboBox->clear();
    _ui->comboBox->clearEditText();
    // Clear status-related items.
    _ui->okLabel->setStatus(OkLabel::Unset);
    _ui->statusLabel->setText("");
    _ui->statusLabel->setStyleSheet("");
    _ui->comboBox->lineEdit()->setStyleSheet("");
}

int PathComboBrowse::count() const
{
    return _ui->comboBox->count();
}

void PathComboBrowse::addItem(const QString &text)
{
    _ui->comboBox->addItem(text);
}

void PathComboBrowse::browseClicked()
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
