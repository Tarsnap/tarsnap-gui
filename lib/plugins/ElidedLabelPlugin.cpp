#include "ElidedLabelPlugin.h"

#include "ElidedLabel.h"

ElidedLabelPlugin::ElidedLabelPlugin(QObject *parent) : QObject(parent)
{
}

QIcon ElidedLabelPlugin::icon() const
{
    return QIcon();
}

QString ElidedLabelPlugin::group() const
{
    return QStringLiteral("Display Widgets");
}

QString ElidedLabelPlugin::includeFile() const
{
    return QStringLiteral("ElidedLabel.h");
}

QString ElidedLabelPlugin::name() const
{
    return QStringLiteral("ElidedLabel");
}

QString ElidedLabelPlugin::toolTip() const
{
    return QString();
}

QString ElidedLabelPlugin::whatsThis() const
{
    return QString();
}

QWidget *ElidedLabelPlugin::createWidget(QWidget *parent)
{
    return new ElidedLabel(parent);
}

bool ElidedLabelPlugin::isContainer() const
{
    return false;
}
