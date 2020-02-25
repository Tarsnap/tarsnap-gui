#include "TPathLineBrowsePlugin.h"

#include "TPathLineBrowse.h"

TPathLineBrowsePlugin::TPathLineBrowsePlugin(QObject *parent) : QObject(parent)
{
}

QIcon TPathLineBrowsePlugin::icon() const
{
    return QIcon();
}

QString TPathLineBrowsePlugin::group() const
{
    return QStringLiteral("Input Widgets");
}

QString TPathLineBrowsePlugin::includeFile() const
{
    return QStringLiteral("TPathLineBrowse.h");
}

QString TPathLineBrowsePlugin::name() const
{
    return QStringLiteral("TPathLineBrowse");
}

QString TPathLineBrowsePlugin::toolTip() const
{
    return QString();
}

QString TPathLineBrowsePlugin::whatsThis() const
{
    return QString();
}

QWidget *TPathLineBrowsePlugin::createWidget(QWidget *parent)
{
    return new TPathLineBrowse(parent);
}

bool TPathLineBrowsePlugin::isContainer() const
{
    return false;
}
