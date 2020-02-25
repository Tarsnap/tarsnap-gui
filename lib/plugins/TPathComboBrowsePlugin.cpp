#include "TPathComboBrowsePlugin.h"

#include "TPathComboBrowse.h"

TPathComboBrowsePlugin::TPathComboBrowsePlugin(QObject *parent)
    : QObject(parent)
{
}

QIcon TPathComboBrowsePlugin::icon() const
{
    return QIcon();
}

QString TPathComboBrowsePlugin::group() const
{
    return QStringLiteral("Input Widgets");
}

QString TPathComboBrowsePlugin::includeFile() const
{
    return QStringLiteral("TPathComboBrowse.h");
}

QString TPathComboBrowsePlugin::name() const
{
    return QStringLiteral("TPathComboBrowse");
}

QString TPathComboBrowsePlugin::toolTip() const
{
    return QString();
}

QString TPathComboBrowsePlugin::whatsThis() const
{
    return QString();
}

QWidget *TPathComboBrowsePlugin::createWidget(QWidget *parent)
{
    return new TPathComboBrowse(parent);
}

bool TPathComboBrowsePlugin::isContainer() const
{
    return false;
}
