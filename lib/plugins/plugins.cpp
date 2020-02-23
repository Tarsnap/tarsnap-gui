#include "plugins.h"

#include "ElidedLabelPlugin.h"
#include "OkLabelPlugin.h"
#include "PathComboBrowsePlugin.h"
#include "PathLineBrowsePlugin.h"
#include "TWizardPagePlugin.h"

TarsnapPlugins::TarsnapPlugins(QObject *parent) : QObject(parent)
{
    widgets.append(new ElidedLabelPlugin(this));
    widgets.append(new OkLabelPlugin(this));
    widgets.append(new PathComboBrowsePlugin(this));
    widgets.append(new PathLineBrowsePlugin(this));
    widgets.append(new TWizardPagePlugin(this));
}

QList<QDesignerCustomWidgetInterface *> TarsnapPlugins::customWidgets() const
{
    return widgets;
}
