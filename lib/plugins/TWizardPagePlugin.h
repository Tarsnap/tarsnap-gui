#ifndef TWIZARDPAGEPLUGIN_H
#define TWIZARDPAGEPLUGIN_H

#include <QObject>
#include <QtUiPlugin/QDesignerCustomWidgetInterface>

class TWizardPagePlugin : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    explicit TWizardPagePlugin(QObject *parent = nullptr);

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

#endif /* !TWIZARDPAGEPLUGIN_H */
