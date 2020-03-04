#include "TTextViewPlugin.h"

#include "TTextView.h"

TTextViewPlugin::TTextViewPlugin(QObject *parent) : QObject(parent)
{
}

QIcon TTextViewPlugin::icon() const
{
    return QIcon();
}

QString TTextViewPlugin::group() const
{
    return QStringLiteral("Display Widgets");
}

QString TTextViewPlugin::includeFile() const
{
    return QStringLiteral("TTextView.h");
}

QString TTextViewPlugin::name() const
{
    return QStringLiteral("TTextView");
}

QString TTextViewPlugin::toolTip() const
{
    return QString();
}

QString TTextViewPlugin::whatsThis() const
{
    return QString();
}

QWidget *TTextViewPlugin::createWidget(QWidget *parent)
{
    return new TTextView(parent);
}

bool TTextViewPlugin::isContainer() const
{
    return false;
}
