#include "plugins.h"

TarsnapPlugins::TarsnapPlugins(QObject *parent) : QObject(parent)
{
}

QList<QDesignerCustomWidgetInterface *> TarsnapPlugins::customWidgets() const
{
    return widgets;
}
