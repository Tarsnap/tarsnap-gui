#include "TOkLabelPlugin.h"

#include "TOkLabel.h"

TOkLabelPlugin::TOkLabelPlugin(QObject *parent) : QObject(parent)
{
}

QIcon TOkLabelPlugin::icon() const
{
    return QIcon();
}

QString TOkLabelPlugin::group() const
{
    return QStringLiteral("Display Widgets");
}

QString TOkLabelPlugin::includeFile() const
{
    return QStringLiteral("TOkLabel.h");
}

QString TOkLabelPlugin::name() const
{
    return QStringLiteral("TOkLabel");
}

QString TOkLabelPlugin::toolTip() const
{
    return QString();
}

QString TOkLabelPlugin::whatsThis() const
{
    return QString();
}

QWidget *TOkLabelPlugin::createWidget(QWidget *parent)
{
    return new TOkLabel(parent);
}

bool TOkLabelPlugin::isContainer() const
{
    return false;
}
