#include "OkLabelPlugin.h"

#include "OkLabel.h"

OkLabelPlugin::OkLabelPlugin(QObject *parent) : QObject(parent)
{
}

QIcon OkLabelPlugin::icon() const
{
    return QIcon();
}

QString OkLabelPlugin::group() const
{
    return QStringLiteral("Display Widgets");
}

QString OkLabelPlugin::includeFile() const
{
    return QStringLiteral("OkLabel.h");
}

QString OkLabelPlugin::name() const
{
    return QStringLiteral("OkLabel");
}

QString OkLabelPlugin::toolTip() const
{
    return QString();
}

QString OkLabelPlugin::whatsThis() const
{
    return QString();
}

QWidget *OkLabelPlugin::createWidget(QWidget *parent)
{
    return new OkLabel(parent);
}

bool OkLabelPlugin::isContainer() const
{
    return false;
}
