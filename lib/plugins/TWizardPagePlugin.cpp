#include "TWizardPagePlugin.h"

#include "TWizardPage.h"

TWizardPagePlugin::TWizardPagePlugin(QObject *parent) : QObject(parent)
{
}

QIcon TWizardPagePlugin::icon() const
{
    return QIcon();
}

QString TWizardPagePlugin::group() const
{
    return QStringLiteral("Containers");
}

QString TWizardPagePlugin::includeFile() const
{
    return QStringLiteral("TWizardPage.h");
}

QString TWizardPagePlugin::name() const
{
    return QStringLiteral("TWizardPage");
}

QString TWizardPagePlugin::toolTip() const
{
    return QString();
}

QString TWizardPagePlugin::whatsThis() const
{
    return QString();
}

QWidget *TWizardPagePlugin::createWidget(QWidget *parent)
{
    return new TWizardPage(parent);
}

bool TWizardPagePlugin::isContainer() const
{
    return true;
}
