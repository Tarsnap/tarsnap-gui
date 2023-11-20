#include "TProgressWidgetPlugin.h"

WARNINGS_DISABLE
#include <QStringLiteral>
WARNINGS_ENABLE

class QWidget;

#include "TProgressWidget.h"

TProgressWidgetPlugin::TProgressWidgetPlugin(QObject *parent) : QObject(parent)
{
}

QIcon TProgressWidgetPlugin::icon() const
{
    return (QIcon());
}

QString TProgressWidgetPlugin::group() const
{
    return (QStringLiteral("Display Widgets"));
}

QString TProgressWidgetPlugin::includeFile() const
{
    return (QStringLiteral("TProgressWidget.h"));
}

QString TProgressWidgetPlugin::name() const
{
    return (QStringLiteral("TProgressWidget"));
}

QString TProgressWidgetPlugin::toolTip() const
{
    return (QString());
}

QString TProgressWidgetPlugin::whatsThis() const
{
    return (QString());
}

QWidget *TProgressWidgetPlugin::createWidget(QWidget *parent)
{
    return (new TProgressWidget(parent));
}

bool TProgressWidgetPlugin::isContainer() const
{
    return (false);
}
