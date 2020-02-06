#ifndef TARSNAPPLUGINS_H
#define TARSNAPPLUGINS_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QList>
#include <QObject>
#include <QtUiPlugin/QDesignerCustomWidgetCollectionInterface>
WARNINGS_ENABLE

class TarsnapPlugins : public QObject,
                       public QDesignerCustomWidgetCollectionInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID
                      "org.tarsnap.QDesignerCustomWidgetCollectionInterface")
    Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)

public:
    TarsnapPlugins(QObject *parent = nullptr);

    QList<QDesignerCustomWidgetInterface *> customWidgets() const override;

private:
    QList<QDesignerCustomWidgetInterface *> widgets;
};

#endif /* !TARSNAPPLUGINS_H */
