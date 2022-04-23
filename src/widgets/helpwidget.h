#ifndef HELPWIDGET_H
#define HELPWIDGET_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QObject>
#include <QString>
#include <QWidget>
WARNINGS_ENABLE

/* Forward declaration(s). */
namespace Ui
{
class HelpWidget;
}
class QEvent;

/*!
 * \ingroup widgets-main
 * \brief The HelpWidget is a QWidget which shows some documentation.
 */
class HelpWidget : public QWidget
{
    Q_OBJECT

public:
    //! Constructor.
    explicit HelpWidget(QWidget *parent = nullptr);
    ~HelpWidget() override;

protected:
    //! Handles translation change of language.
    void changeEvent(QEvent *event) override;

private:
    Ui::HelpWidget *_ui;
    QString         _helpTabHTML;

    void updateKeyboardShortcutInfo();
};

#endif // HELPWIDGET_H
