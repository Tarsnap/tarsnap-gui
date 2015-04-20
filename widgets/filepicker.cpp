#include "filepicker.h"
#include "debug.h"

#include <QSettings>
#include <QKeyEvent>
#include <QPersistentModelIndex>

FilePicker::FilePicker(QWidget *parent, QString startPath) :
    QWidget(parent),
    _ui(new Ui::FilePicker),
    _startPath(startPath)
{
    _ui->setupUi(this);
    _ui->optionsContainer->hide();

    _model.setRootPath(QDir::rootPath());
    _model.setNameFilterDisables(false);
    _ui->treeView->setModel(&_model);
    if(_startPath.isEmpty())
    {
        QString path;
        QSettings settings;
        path = settings.value("app/file_browse_last", QDir::homePath()).toString();
        _ui->treeView->setCurrentIndex(_model.index(path));
    }
    else
    {
        _ui->treeView->setCurrentIndex(_model.index(_startPath));
    }
    _completer.setModel(&_model);
    _completer.setCompletionMode(QCompleter::InlineCompletion);
    _completer.setCaseSensitivity(Qt::CaseSensitive);
    _ui->filterLineEdit->setCompleter(&_completer);
    _ui->treeView->setColumnWidth(0, 250);
    _ui->filterLineEdit->setFocus();

    connect(&_model, &CustomFileSystemModel::dataChanged,
            [=](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
            {
                Q_UNUSED(topLeft);Q_UNUSED(bottomRight);
                if(!roles.isEmpty() && roles.first() == Qt::CheckStateRole)
                    emit selectionChanged();
            });
    connect(_ui->filterLineEdit, SIGNAL(textEdited(QString)), this, SLOT(updateFilter(QString)));
    connect(_ui->showHiddenCheckBox, &QCheckBox::toggled,
            [=](const bool toggled)
            {
                if(toggled)
                    _model.setFilter(_model.filter() | QDir::Hidden);
                else
                    _model.setFilter(_model.filter() & ~QDir::Hidden);
            });
    connect(_ui->showSystemCheckBox, &QCheckBox::toggled,
            [=](const bool toggled)
            {
                if(toggled)
                    _model.setFilter(_model.filter() | QDir::System);
                else
                    _model.setFilter(_model.filter() & ~QDir::System);
            });
    connect(_ui->hideLinksCheckBox, &QCheckBox::toggled,
            [=](const bool toggled)
            {
                if(toggled)
                    _model.setFilter(_model.filter() | QDir::NoSymLinks);
                else
                    _model.setFilter(_model.filter() & ~QDir::NoSymLinks);
            });
    connect(_ui->showOptionsButton, &QPushButton::clicked,
            [=]()
            {
                _ui->optionsContainer->setVisible(!_ui->optionsContainer->isVisible());
            });
    connect(_ui->filterLineEdit, &QLineEdit::returnPressed,
            [=]()
            {
                if(_completer.currentCompletion().isEmpty())
                    _ui->treeView->setFocus();
            });
}

FilePicker::~FilePicker()
{
    if(_startPath.isEmpty())
    {
        QSettings settings;
        settings.setValue("app/file_browse_last", _model.filePath(_ui->treeView->currentIndex()));
    }
    delete _ui;
}

void FilePicker::reset()
{
    _model.reset();
    _ui->treeView->reset();
}

QList<QUrl> FilePicker::getSelectedUrls()
{
    QList<QUrl> urls;
    QList<QPersistentModelIndex> indexList = _model.checkedIndexes();
    foreach(QPersistentModelIndex index, indexList)
        urls << QUrl::fromUserInput(_model.filePath(index));
    return urls;
}

void FilePicker::setSelectedUrls(const QList<QUrl> &urls)
{
    foreach(const QUrl url, urls)
    {
        _model.setData(_model.index(url.toLocalFile()), Qt::Checked, Qt::CheckStateRole);
    }
}

void FilePicker::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Escape:
        if(_ui->optionsContainer->isVisible())
            _ui->optionsContainer->hide();
        else
            _ui->filterLineEdit->setFocus();
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void FilePicker::updateFilter(QString filter)
{
    if(filter.isEmpty())
        _model.setNameFilters(QStringList("*"));
    else if(filter.startsWith('/'))
    {
        _ui->treeView->setCurrentIndex(_model.index(filter));
        _ui->treeView->scrollTo(_model.index(filter));
    }
    else
        _model.setNameFilters(QStringList(filter));
}
