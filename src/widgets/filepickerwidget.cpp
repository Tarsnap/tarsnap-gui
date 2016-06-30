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

    // Configure underlying model.
    _model.setRootPath(QDir::rootPath());

    // Configure file url completions.
    _completer.setModel(&_model);
    _completer.setCompletionMode(QCompleter::InlineCompletion);
    _completer.setCaseSensitivity(Qt::CaseSensitive);
    _ui.filterLineEdit->setCompleter(&_completer);
    _ui.filterLineEdit->setFocus();

    // Configure visual display of the model.
    _ui.treeView->setModel(&_model);
    _ui.treeView->installEventFilter(this);
    _ui.treeView->setColumnWidth(0, 250);

    // Load last browsed file url.
    QSettings settings;
    setCurrentPath(
        settings.value("app/file_browse_last", QDir::homePath()).toString());

    // Connection for the model's data changing.
    connect(&_model, &CustomFileSystemModel::dataChanged,
            [&](const QModelIndex &topLeft, const QModelIndex &bottomRight,
                const QVector<int> &roles) {
                Q_UNUSED(topLeft);
                Q_UNUSED(bottomRight);
                if(!roles.isEmpty() && (roles.first() == SELECTION_CHANGED_ROLE))
                    emit selectionChanged();
            });
    // Connections for the top bar of the widget
    connect(_ui.showOptionsButton, &QPushButton::clicked, [&]() {
        _ui.optionsContainer->setVisible(!_ui.optionsContainer->isVisible());
    });
    connect(_ui.homeButton, &QPushButton::clicked,
            [&]() { setCurrentPath(QDir::homePath()); });
    connect(_ui.filterLineEdit, &QLineEdit::textEdited, this,
            &FilePickerWidget::updateFilter);
    connect(_ui.filterLineEdit, &QLineEdit::returnPressed, [&]() {
        if(_completer.currentCompletion().isEmpty())
            _ui.treeView->setFocus();
    });
    // Connections for the settings
    connect(_ui.showHiddenCheckBox, &QCheckBox::toggled, [&](const bool toggled) {
        if(toggled)
            _model.setFilter(_model.filter() | QDir::Hidden);
        else
            _model.setFilter(_model.filter() & ~QDir::Hidden);
        emit settingChanged();
    });
    connect(_ui.showSystemCheckBox, &QCheckBox::toggled, [&](const bool toggled) {
        if(toggled)
            _model.setFilter(_model.filter() | QDir::System);
        else
            _model.setFilter(_model.filter() & ~QDir::System);
        emit settingChanged();
    });
    connect(_ui.hideLinksCheckBox, &QCheckBox::toggled, [&](const bool toggled) {
        if(toggled)
            _model.setFilter(_model.filter() | QDir::NoSymLinks);
        else
            _model.setFilter(_model.filter() & ~QDir::NoSymLinks);
        emit settingChanged();
    });
}

FilePickerWidget::~FilePickerWidget()
{
}

void FilePickerWidget::reset()
{
    _model.reset();
    _ui.treeView->reset();
    // Load last browsed file url.
    QSettings settings;
    setCurrentPath(
        settings.value("app/file_browse_last", QDir::homePath()).toString());
}

QList<QUrl> FilePickerWidget::getSelectedUrls()
{
    // Construct a list of urls from the filePath QStrings.
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

bool FilePickerWidget::settingShowHidden()
{
    return _ui.showHiddenCheckBox->isChecked();
}

void FilePickerWidget::setSettingShowHidden(bool showHidden)
{
    _ui.showHiddenCheckBox->setChecked(showHidden);
}

bool FilePickerWidget::settingShowSystem()
{
    return _ui.showSystemCheckBox->isChecked();
}

void FilePickerWidget::setSettingShowSystem(bool showSystem)
{
    _ui.showSystemCheckBox->setChecked(showSystem);
}

bool FilePickerWidget::settingHideSymlinks()
{
    return _ui.hideLinksCheckBox->isChecked();
}

void FilePickerWidget::setSettingHideSymlinks(bool hideSymlinks)
{
    _ui.hideLinksCheckBox->setChecked(hideSymlinks);
}

void FilePickerWidget::keyPressEvent(QKeyEvent *event)
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
            QWidget::keyPressEvent(event);
        }
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

bool FilePickerWidget::eventFilter(QObject *obj, QEvent *event)
{
    if((obj == _ui.treeView) && (event->type() == QEvent::FocusOut))
    {
        // Save last browsed file url.
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
