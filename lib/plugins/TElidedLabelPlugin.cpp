#include "TElidedLabelPlugin.h"

#include "TElidedLabel.h"

TElidedLabelPlugin::TElidedLabelPlugin(QObject *parent) : QObject(parent)
{
}

QIcon TElidedLabelPlugin::icon() const
{
    return QIcon();
}

QString TElidedLabelPlugin::group() const
{
    return QStringLiteral("Display Widgets");
}

QString TElidedLabelPlugin::includeFile() const
{
    return QStringLiteral("TElidedLabel.h");
}

QString TElidedLabelPlugin::name() const
{
    return QStringLiteral("TElidedLabel");
}

QString TElidedLabelPlugin::toolTip() const
{
    return QString();
}

QString TElidedLabelPlugin::whatsThis() const
{
    return QString();
}

QWidget *TElidedLabelPlugin::createWidget(QWidget *parent)
{
    return new TElidedLabel(parent);
}

bool TElidedLabelPlugin::isContainer() const
{
    return false;
}
