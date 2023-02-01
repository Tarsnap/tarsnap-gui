#include "TTabWidgetPlugin.h"

WARNINGS_DISABLE
#include <QStringLiteral>
WARNINGS_ENABLE

class QWidget;

#include "TTabWidget.h"

TTabWidgetPlugin::TTabWidgetPlugin(QObject *parent)
    : QObject(parent), _initialized(false)
{
}

QIcon TTabWidgetPlugin::icon() const
{
    return (QIcon());
}

QString TTabWidgetPlugin::group() const
{
    return (QStringLiteral("Containers"));
}

QString TTabWidgetPlugin::includeFile() const
{
    return (QStringLiteral("TTabWidget.h"));
}

QString TTabWidgetPlugin::name() const
{
    return (QStringLiteral("TTabWidget"));
}

QString TTabWidgetPlugin::toolTip() const
{
    return (QString());
}

QString TTabWidgetPlugin::whatsThis() const
{
    return (QString());
}

QWidget *TTabWidgetPlugin::createWidget(QWidget *parent)
{
    return (new TTabWidget(parent));
}

bool TTabWidgetPlugin::isContainer() const
{
    return (false);
}
