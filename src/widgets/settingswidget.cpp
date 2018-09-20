#include "settingswidget.h"

#include <QWidget>

SettingsWidget::SettingsWidget(QWidget *parent) : QWidget(parent)
{

    // Ui initialization
    _ui.setupUi(this);

    // Settings
    loadSettings();
}

void SettingsWidget::initializeSettingsWidget()
{
    QSettings settings;
}

void SettingsWidget::loadSettings()
{
    QSettings settings;

}

void SettingsWidget::commitSettings()
{
    QSettings settings;

    settings.sync();
}

void SettingsWidget::updateUi()
{
}
