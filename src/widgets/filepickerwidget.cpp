#include "filepickerwidget.h"

WARNINGS_DISABLE
#include <QCheckBox>
#include <QCompleter>
#include <QDir>
#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPersistentModelIndex>
#include <QPushButton>
#include <QStringList>
#include <QTreeView>
#include <QUrl>
#include <QVector>
#include <Qt>

#include "ui_filepickerwidget.h"
WARNINGS_ENABLE

#include "customfilesystemmodel.h"

/* Forward declaration(s). */
class QModelIndex;

FilePickerWidget::FilePickerWidget(QWidget *parent)
    : QWidget(parent),
      _ui(new Ui::FilePickerWidget),
      _model(new CustomFileSystemModel),
      _completer(new QCompleter)
{
    _ui->setupUi(this);
    _ui->optionsContainer->hide();

    // Configure underlying model.
    _model->setRootPath(QDir::rootPath());

    // Configure file url completions.
    _completer->setModel(_model);
    _completer->setCompletionMode(QCompleter::InlineCompletion);
    _completer->setCaseSensitivity(Qt::CaseSensitive);
    _ui->filterLineEdit->setCompleter(_completer);
    _ui->filterLineEdit->setFocus();

    // Configure visual display of the model.
    _ui->treeView->setModel(_model);
    _ui->treeView->setColumnWidth(0, 250);

    // Select the home directory in the display.
    setCurrentPath(QDir::homePath());

    // Connection for the model's data changing.
    connect(_model, &CustomFileSystemModel::dataChanged,
            [this](const QModelIndex &topLeft, const QModelIndex &bottomRight,
                   const QVector<int> &roles) {
                Q_UNUSED(topLeft);
                Q_UNUSED(bottomRight);
                if(!roles.isEmpty()
                   && (roles.first() == SELECTION_CHANGED_ROLE))
                {
                    emit selectionChanged();
                    _ui->treeView->viewport()->update();
                }
            });
    // Connections for the top bar of the widget
    connect(_ui->showOptionsButton, &QPushButton::clicked, [this]() {
        _ui->optionsContainer->setVisible(!_ui->optionsContainer->isVisible());
    });
    connect(_ui->homeButton, &QPushButton::clicked,
            [this]() { setCurrentPath(QDir::homePath()); });
    connect(_ui->filterLineEdit, &QLineEdit::textEdited, this,
            &FilePickerWidget::updateFilter);
    connect(_ui->filterLineEdit, &QLineEdit::returnPressed, [this]() {
        if(_completer->currentCompletion().isEmpty())
            _ui->treeView->setFocus();
    });
    // Connections for the settings
    connect(_ui->showHiddenCheckBox, &QCheckBox::toggled,
            [this](const bool toggled) {
                if(toggled)
                    _model->setFilter(_model->filter() | QDir::Hidden);
                else
                    _model->setFilter(_model->filter() & ~QDir::Hidden);
                emit settingChanged();
            });
    connect(_ui->showSystemCheckBox, &QCheckBox::toggled,
            [this](const bool toggled) {
                if(toggled)
                    _model->setFilter(_model->filter() | QDir::System);
                else
                    _model->setFilter(_model->filter() & ~QDir::System);
                emit settingChanged();
            });
    connect(_ui->hideLinksCheckBox, &QCheckBox::toggled,
            [this](const bool toggled) {
                if(toggled)
                    _model->setFilter(_model->filter() | QDir::NoSymLinks);
                else
                    _model->setFilter(_model->filter() & ~QDir::NoSymLinks);
                emit settingChanged();
            });
}

FilePickerWidget::~FilePickerWidget()
{
    delete _completer;
    delete _model;
    delete _ui;
}

void FilePickerWidget::reset()
{
    _model->reset();
    _ui->treeView->reset();
    // Select the home directory in the display.
    setCurrentPath(QDir::homePath());
}

QString FilePickerWidget::getCurrentPath()
{
    return _model->filePath(_ui->treeView->currentIndex());
}

QList<QUrl> FilePickerWidget::getSelectedUrls()
{
    // Construct a list of urls from the filePath QStrings.
    QList<QUrl>                  urls;
    QList<QPersistentModelIndex> indexList = _model->checkedIndexes();
    for(const QPersistentModelIndex &index : indexList)
        urls << QUrl::fromUserInput(_model->filePath(index));
    return urls;
}

void FilePickerWidget::setSelectedUrls(const QList<QUrl> &urls)
{
    _model->reset();
    for(const QUrl &url : urls)
    {
        _model->setData(_model->index(url.toLocalFile()), Qt::Checked,
                        Qt::CheckStateRole);
    }
}

void FilePickerWidget::selectUrl(const QUrl &url)
{
    _model->setData(_model->index(url.toLocalFile()), Qt::Checked,
                    Qt::CheckStateRole);
}

bool FilePickerWidget::settingShowHidden()
{
    return _ui->showHiddenCheckBox->isChecked();
}

void FilePickerWidget::setSettingShowHidden(bool showHidden)
{
    _ui->showHiddenCheckBox->setChecked(showHidden);
}

bool FilePickerWidget::settingShowSystem()
{
    return _ui->showSystemCheckBox->isChecked();
}

void FilePickerWidget::setSettingShowSystem(bool showSystem)
{
    _ui->showSystemCheckBox->setChecked(showSystem);
}

bool FilePickerWidget::settingHideSymlinks()
{
    return _ui->hideLinksCheckBox->isChecked();
}

void FilePickerWidget::setSettingHideSymlinks(bool hideSymlinks)
{
    _ui->hideLinksCheckBox->setChecked(hideSymlinks);
}

void FilePickerWidget::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Escape:
        if(_ui->optionsContainer->isVisible())
        {
            _ui->optionsContainer->hide();
        }
        else if(!_ui->filterLineEdit->hasFocus())
        {
            _ui->filterLineEdit->setFocus();
        }
        else
        {
            QWidget::keyPressEvent(event);
        }
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void FilePickerWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
        _ui->retranslateUi(this);
    QWidget::changeEvent(event);
}

void FilePickerWidget::updateFilter(const QString &filter)
{
    _model->setNameFilters(QStringList("*"));
    if(filter.startsWith('/'))
        setCurrentPath(filter);
    else if(!filter.isEmpty())
        _model->setNameFilters(QStringList(filter));
}

void FilePickerWidget::setCurrentPath(const QString &path)
{
    _ui->treeView->setCurrentIndex(_model->index(path));
    _ui->treeView->scrollTo(_model->index(path));
}
