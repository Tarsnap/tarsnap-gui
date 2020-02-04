#include "plugins.h"

#include "ElidedLabelPlugin.h"

TarsnapPlugins::TarsnapPlugins(QObject *parent) : QObject(parent)
{
    widgets.append(new ElidedLabelPlugin(this));
}

QList<QDesignerCustomWidgetInterface *> TarsnapPlugins::customWidgets() const
{
    return widgets;
}
