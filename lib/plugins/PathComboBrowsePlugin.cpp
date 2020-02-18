#include "PathComboBrowsePlugin.h"

#include "PathComboBrowse.h"

PathComboBrowsePlugin::PathComboBrowsePlugin(QObject *parent) : QObject(parent)
{
}

QIcon PathComboBrowsePlugin::icon() const
{
    return QIcon();
}

QString PathComboBrowsePlugin::group() const
{
    return QStringLiteral("Input Widgets");
}

QString PathComboBrowsePlugin::includeFile() const
{
    return QStringLiteral("PathComboBrowse.h");
}

QString PathComboBrowsePlugin::name() const
{
    return QStringLiteral("PathComboBrowse");
}

QString PathComboBrowsePlugin::toolTip() const
{
    return QString();
}

QString PathComboBrowsePlugin::whatsThis() const
{
    return QString();
}

QWidget *PathComboBrowsePlugin::createWidget(QWidget *parent)
{
    return new PathComboBrowse(parent);
}

bool PathComboBrowsePlugin::isContainer() const
{
    return false;
}
