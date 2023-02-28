#include "TTabWidgetPlugin.h"

WARNINGS_DISABLE
#include <QStringLiteral>
WARNINGS_ENABLE

class QWidget;

#include "TTabWidget.h"

const static char *TTabWidget_Xml = R"""(
<ui language="c++">
 <widget class="TTabWidget" name="ttabWidget">
 </widget>
 <customwidgets>
  <customwidget>
   <class>TTabWidget</class>
   <extends>QTabWidget</extends>
   <propertyspecifications>
    <stringpropertyspecification name="largeLogoFilename" notr="true" type="url" />
    <stringpropertyspecification name="smallLogoFilename" notr="true" type="url" />
   </propertyspecifications>
  </customwidget>
 </customwidgets>
</ui>)""";

TTabWidgetPlugin::TTabWidgetPlugin(QObject *parent) : QObject(parent)
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
    return (true);
}

QString TTabWidgetPlugin::domXml() const
{
    return (TTabWidget_Xml);
}
