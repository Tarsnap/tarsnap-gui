#include "TPopupPushButtonPlugin.h"

#include "TPopupPushButton.h"

TPopupPushButtonPlugin::TPopupPushButtonPlugin(QObject *parent)
    : QObject(parent)
{
}

QIcon TPopupPushButtonPlugin::icon() const
{
    return QIcon();
}

QString TPopupPushButtonPlugin::group() const
{
    return QStringLiteral("Display Widgets");
}

QString TPopupPushButtonPlugin::includeFile() const
{
    return QStringLiteral("TPopupPushButton.h");
}

QString TPopupPushButtonPlugin::name() const
{
    return QStringLiteral("TPopupPushButton");
}

QString TPopupPushButtonPlugin::toolTip() const
{
    return QString();
}

QString TPopupPushButtonPlugin::whatsThis() const
{
    return QString();
}

QWidget *TPopupPushButtonPlugin::createWidget(QWidget *parent)
{
    return new TPopupPushButton(parent);
}

bool TPopupPushButtonPlugin::isContainer() const
{
    return false;
}
