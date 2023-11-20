#include "TAsideLabelPlugin.h"

WARNINGS_DISABLE
#include <QStringLiteral>
WARNINGS_ENABLE

class QWidget;

#include "TAsideLabel.h"

TAsideLabelPlugin::TAsideLabelPlugin(QObject *parent) : QObject(parent)
{
}

QIcon TAsideLabelPlugin::icon() const
{
    return (QIcon());
}

QString TAsideLabelPlugin::group() const
{
    return (QStringLiteral("Display Widgets"));
}

QString TAsideLabelPlugin::includeFile() const
{
    return (QStringLiteral("TAsideLabel.h"));
}

QString TAsideLabelPlugin::name() const
{
    return (QStringLiteral("TAsideLabel"));
}

QString TAsideLabelPlugin::toolTip() const
{
    return (QString());
}

QString TAsideLabelPlugin::whatsThis() const
{
    return (QString());
}

QWidget *TAsideLabelPlugin::createWidget(QWidget *parent)
{
    return (new TAsideLabel(parent));
}

bool TAsideLabelPlugin::isContainer() const
{
    return (false);
}
