#include "plugins.h"

#include "ElidedLabelPlugin.h"
#include "PathLineBrowsePlugin.h"

TarsnapPlugins::TarsnapPlugins(QObject *parent) : QObject(parent)
{
    widgets.append(new ElidedLabelPlugin(this));
    widgets.append(new PathLineBrowsePlugin(this));
}

QList<QDesignerCustomWidgetInterface *> TarsnapPlugins::customWidgets() const
{
    return widgets;
}
