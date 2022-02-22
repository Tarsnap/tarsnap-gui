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
class TTextView;
class QDialog;
class QEvent;

/*!
 * \ingroup widgets-main
 * \brief The HelpWidget is a QWidget which shows some documentation.
 */
class HelpWidget : public QWidget
{
    Q_OBJECT

#ifdef QT_TESTLIB_LIB
    friend class TestMainWindow;
    friend class TestHelpWidget;
#endif

public:
    //! Constructor.
    explicit HelpWidget(QWidget *parent = nullptr);
    ~HelpWidget() override;

public slots:
    //! The "about" app menu item was clicked.
    void aboutMenuClicked();

    //! Append a log message.
    void appendLogString(const QString &text);

protected:
    //! Handles translation change of language.
    void changeEvent(QEvent *event) override;

private:
    Ui::HelpWidget *_ui;
    QString         _helpTabHTML;
    QDialog        *_aboutWindow;
    QDialog        *_consoleWindow;

    TTextView *_consoleLog;

    void updateKeyboardShortcutInfo();
};

#endif // HELPWIDGET_H
