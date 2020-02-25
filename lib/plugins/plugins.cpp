#include "plugins.h"

#include "ElidedLabelPlugin.h"
#include "OkLabelPlugin.h"
#include "TPathComboBrowsePlugin.h"
#include "TPathLineBrowsePlugin.h"
#include "TWizardPagePlugin.h"

TarsnapPlugins::TarsnapPlugins(QObject *parent) : QObject(parent)
{
    widgets.append(new ElidedLabelPlugin(this));
    widgets.append(new OkLabelPlugin(this));
    widgets.append(new TPathComboBrowsePlugin(this));
    widgets.append(new TPathLineBrowsePlugin(this));
    widgets.append(new TWizardPagePlugin(this));
}

QList<QDesignerCustomWidgetInterface *> TarsnapPlugins::customWidgets() const
{
    return widgets;
}
