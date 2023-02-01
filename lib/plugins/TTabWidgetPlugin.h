#ifndef TTABWIGETPLUGIN_H
#define TTABWIGETPLUGIN_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QDesignerCustomWidgetInterface>
#include <QIcon>
#include <QObject>
#include <QString>
WARNINGS_ENABLE

/* Forward declaration(s). */
class QWidget;

class TTabWidgetPlugin : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    explicit TTabWidgetPlugin(QObject *parent = nullptr);

    QIcon    icon() const override;
    QString  group() const override;
    QString  includeFile() const override;
    QString  name() const override;
    QString  toolTip() const override;
    QString  whatsThis() const override;
    QWidget *createWidget(QWidget *parent) override;
    bool     isContainer() const override;

private:
    bool _initialized;
};

#endif /* !TTABWIGETPLUGIN_H */
