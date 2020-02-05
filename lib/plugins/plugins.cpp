#include "plugins.h"

#include "ElidedLabelPlugin.h"
#include "PathLineBrowsePlugin.h"
#include "TWizardPagePlugin.h"

TarsnapPlugins::TarsnapPlugins(QObject *parent) : QObject(parent)
{
    widgets.append(new ElidedLabelPlugin(this));
    widgets.append(new PathLineBrowsePlugin(this));
    widgets.append(new TWizardPagePlugin(this));
}

QList<QDesignerCustomWidgetInterface *> TarsnapPlugins::customWidgets() const
{
    return widgets;
}
