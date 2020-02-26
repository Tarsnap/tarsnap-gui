#include "TBusyLabelPlugin.h"

#include "TBusyLabel.h"

TBusyLabelPlugin::TBusyLabelPlugin(QObject *parent) : QObject(parent)
{
}

QIcon TBusyLabelPlugin::icon() const
{
    return QIcon();
}

QString TBusyLabelPlugin::group() const
{
    return QStringLiteral("Display Widgets");
}

QString TBusyLabelPlugin::includeFile() const
{
    return QStringLiteral("TBusyLabel.h");
}

QString TBusyLabelPlugin::name() const
{
    return QStringLiteral("TBusyLabel");
}

QString TBusyLabelPlugin::toolTip() const
{
    return QString();
}

QString TBusyLabelPlugin::whatsThis() const
{
    return QString();
}

QWidget *TBusyLabelPlugin::createWidget(QWidget *parent)
{
    return new TBusyLabel(parent);
}

bool TBusyLabelPlugin::isContainer() const
{
    return false;
}
