#include "TOkLabelPlugin.h"

WARNINGS_DISABLE
#include <QStringLiteral>
WARNINGS_ENABLE

class QWidget;

#include "TOkLabel.h"

TOkLabelPlugin::TOkLabelPlugin(QObject *parent) : QObject(parent)
{
}

QIcon TOkLabelPlugin::icon() const
{
    return (QIcon(":lib/TOkYesNo.png"));
}

QString TOkLabelPlugin::group() const
{
    return (QStringLiteral("Display Widgets"));
}

QString TOkLabelPlugin::includeFile() const
{
    return (QStringLiteral("TOkLabel.h"));
}

QString TOkLabelPlugin::name() const
{
    return (QStringLiteral("TOkLabel"));
}

QString TOkLabelPlugin::toolTip() const
{
    return (QString());
}

QString TOkLabelPlugin::whatsThis() const
{
    return (QString());
}

QWidget *TOkLabelPlugin::createWidget(QWidget *parent)
{
    return (new TOkLabel(parent));
}

bool TOkLabelPlugin::isContainer() const
{
    return (false);
}
