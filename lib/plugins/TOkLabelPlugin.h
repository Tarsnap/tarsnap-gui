#ifndef TOKLABELPLUGIN_H
#define TOKLABELPLUGIN_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QObject>
#include <QtUiPlugin/QDesignerCustomWidgetInterface>
WARNINGS_ENABLE

class TOkLabelPlugin : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    explicit TOkLabelPlugin(QObject *parent = nullptr);

    QIcon    icon() const override;
    QString  group() const override;
    QString  includeFile() const override;
    QString  name() const override;
    QString  toolTip() const override;
    QString  whatsThis() const override;
    QWidget *createWidget(QWidget *parent) override;
    bool     isContainer() const override;

private:
    bool initialized = false;
};

#endif /* !TOKLABELPLUGIN_H */
