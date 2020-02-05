#include "PathLineBrowse.h"

#include <QFileDialog>

#include "ui_PathLineBrowse.h"

PathLineBrowse::PathLineBrowse(QWidget *parent)
    : QWidget(parent), _ui(new Ui::PathLineBrowse)
{
    _ui->setupUi(this);

    // We want to allow the combined widget to accept the focus (in
    // the UI), but that focus should be redirected to the lineEdit.
    setFocusProxy(_ui->lineEdit);

    // External connections
    connect(_ui->lineEdit, &QLineEdit::textChanged, this,
            &PathLineBrowse::textChanged);

    // Internal connections
    connect(_ui->button, &QPushButton::clicked, this,
            &PathLineBrowse::browseClicked);
}

PathLineBrowse::~PathLineBrowse()
{
    delete _ui;
}

QString PathLineBrowse::label() const
{
    return _ui->label->text();
}

void PathLineBrowse::setLabel(const QString &text)
{
    _ui->label->setText(text);
}

QString PathLineBrowse::placeholderText() const
{
    return _ui->lineEdit->placeholderText();
}

void PathLineBrowse::setPlaceholderText(const QString &text)
{
    _ui->lineEdit->setPlaceholderText(text);
}

QString PathLineBrowse::dialogTitle() const
{
    return _dialogTitle;
}

void PathLineBrowse::setDialogTitle(const QString &text)
{
    _dialogTitle = text;
}

QString PathLineBrowse::text() const
{
    return _ui->lineEdit->text();
}

void PathLineBrowse::setText(const QString &text)
{
    _ui->lineEdit->setText(text);
}

void PathLineBrowse::browseClicked()
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
