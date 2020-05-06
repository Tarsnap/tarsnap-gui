#include "plugins.h"

#include "TBusyLabelPlugin.h"
#include "TElidedLabelPlugin.h"
#include "TOkLabelPlugin.h"
#include "TPathComboBrowsePlugin.h"
#include "TPathLineBrowsePlugin.h"
#include "TPopupPushButtonPlugin.h"
#include "TTextViewPlugin.h"
#include "TWizardPagePlugin.h"

TarsnapPlugins::TarsnapPlugins(QObject *parent) : QObject(parent)
{
    widgets.append(new TBusyLabelPlugin(this));
    widgets.append(new TElidedLabelPlugin(this));
    widgets.append(new TOkLabelPlugin(this));
    widgets.append(new TPathComboBrowsePlugin(this));
    widgets.append(new TPathLineBrowsePlugin(this));
    widgets.append(new TPopupPushButtonPlugin(this));
    widgets.append(new TTextViewPlugin(this));
    widgets.append(new TWizardPagePlugin(this));
}

QList<QDesignerCustomWidgetInterface *> TarsnapPlugins::customWidgets() const
{
    return widgets;
}
