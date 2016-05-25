#include "filepickerwidget.h"

#include "debug.h"

#include <QKeyEvent>
#include <QPersistentModelIndex>
#include <QSettings>

FilePickerWidget::FilePickerWidget(QWidget *parent)
    : QWidget(parent)
{
    _ui.setupUi(this);
    _ui.optionsContainer->hide();

    _model.setRootPath(QDir::rootPath());
    //    _model.setNameFilterDisables(false);
    _ui.treeView->setModel(&_model);
    _ui.treeView->installEventFilter(this);
    QSettings settings;
    setCurrentPath(
        settings.value("app/file_browse_last", QDir::homePath()).toString());
    _completer.setModel(&_model);
    _completer.setCompletionMode(QCompleter::InlineCompletion);
    _completer.setCaseSensitivity(Qt::CaseSensitive);
    _ui.filterLineEdit->setCompleter(&_completer);
    _ui.treeView->setColumnWidth(0, 250);
    _ui.filterLineEdit->setFocus();

    connect(&_model, &CustomFileSystemModel::dataChanged,
            [&](const QModelIndex &topLeft, const QModelIndex &bottomRight,
                const QVector<int> &roles) {
                Q_UNUSED(topLeft);
                Q_UNUSED(bottomRight);
                if(!roles.isEmpty() && (roles.first() == Qt::CheckStateRole))
                    emit selectionChanged();
            });
    connect(_ui.filterLineEdit, &QLineEdit::textEdited, this,
            &FilePickerWidget::updateFilter);
    connect(_ui.showHiddenCheckBox, &QCheckBox::toggled, [&](const bool toggled) {
        if(toggled)
            _model.setFilter(_model.filter() | QDir::Hidden);
        else
            _model.setFilter(_model.filter() & ~QDir::Hidden);
    });
    connect(_ui.showSystemCheckBox, &QCheckBox::toggled, [&](const bool toggled) {
        if(toggled)
            _model.setFilter(_model.filter() | QDir::System);
        else
            _model.setFilter(_model.filter() & ~QDir::System);
    });
    connect(_ui.hideLinksCheckBox, &QCheckBox::toggled, [&](const bool toggled) {
        if(toggled)
            _model.setFilter(_model.filter() | QDir::NoSymLinks);
        else
            _model.setFilter(_model.filter() & ~QDir::NoSymLinks);
    });
    connect(_ui.showOptionsButton, &QPushButton::clicked, [&]() {
        _ui.optionsContainer->setVisible(!_ui.optionsContainer->isVisible());
    });
    connect(_ui.filterLineEdit, &QLineEdit::returnPressed, [&]() {
        if(_completer.currentCompletion().isEmpty())
            _ui.treeView->setFocus();
    });
    connect(_ui.homeButton, &QPushButton::clicked,
            [&]() { setCurrentPath(QDir::homePath()); });
}

FilePickerWidget::~FilePickerWidget()
{
}

void FilePickerWidget::reset()
{
    _model.reset();
    _ui.treeView->reset();
    QSettings settings;
    setCurrentPath(
        settings.value("app/file_browse_last", QDir::homePath()).toString());
}

QList<QUrl> FilePickerWidget::getSelectedUrls()
{
    QList<QUrl> urls;
    QList<QPersistentModelIndex> indexList = _model.checkedIndexes();
    foreach(QPersistentModelIndex index, indexList)
        urls << QUrl::fromUserInput(_model.filePath(index));
    return urls;
}

void FilePickerWidget::setSelectedUrls(const QList<QUrl> &urls)
{
    _model.reset();
    foreach(const QUrl url, urls)
    {
        _model.setData(_model.index(url.toLocalFile()), Qt::Checked,
                       Qt::CheckStateRole);
    }
}

void FilePickerWidget::selectUrl(QUrl url)
{
    _model.setData(_model.index(url.toLocalFile()), Qt::Checked,
                   Qt::CheckStateRole);
}

void FilePickerWidget::keyReleaseEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Escape:
        if(_ui.optionsContainer->isVisible())
        {
            _ui.optionsContainer->hide();
        }
        else if(!_ui.filterLineEdit->hasFocus())
        {
            _ui.filterLineEdit->setFocus();
        }
        else
        {
            QWidget::keyReleaseEvent(event);
        }
        break;
    default:
        QWidget::keyReleaseEvent(event);
    }
}

bool FilePickerWidget::eventFilter(QObject *obj, QEvent *event)
{
    if((obj == _ui.treeView) && (event->type() == QEvent::FocusOut))
    {
        emit focusLost();
        QSettings settings;
        settings.setValue("app/file_browse_last",
                          _model.filePath(_ui.treeView->currentIndex()));
        return false;
    }
    else
    {
        // standard event processing
        return QWidget::eventFilter(obj, event);
    }
}

void FilePickerWidget::updateFilter(QString filter)
{
    _model.setNameFilters(QStringList("*"));
    if(filter.startsWith('/'))
        setCurrentPath(filter);
    else if(!filter.isEmpty())
        _model.setNameFilters(QStringList(filter));
}

void FilePickerWidget::setCurrentPath(const QString path)
{
    _ui.treeView->setCurrentIndex(_model.index(path));
    _ui.treeView->scrollTo(_model.index(path));
}
