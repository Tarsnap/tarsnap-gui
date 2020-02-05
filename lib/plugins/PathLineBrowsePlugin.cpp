#include "PathLineBrowsePlugin.h"

#include "PathLineBrowse.h"

PathLineBrowsePlugin::PathLineBrowsePlugin(QObject *parent) : QObject(parent)
{
}

QIcon PathLineBrowsePlugin::icon() const
{
    return QIcon();
}

QString PathLineBrowsePlugin::group() const
{
    return QStringLiteral("Input Widgets");
}

QString PathLineBrowsePlugin::includeFile() const
{
    return QStringLiteral("PathLineBrowse.h");
}

QString PathLineBrowsePlugin::name() const
{
    return QStringLiteral("PathLineBrowse");
}

QString PathLineBrowsePlugin::toolTip() const
{
    return QString();
}

QString PathLineBrowsePlugin::whatsThis() const
{
    return QString();
}

QWidget *PathLineBrowsePlugin::createWidget(QWidget *parent)
{
    return new PathLineBrowse(parent);
}

bool PathLineBrowsePlugin::isContainer() const
{
    return false;
}
