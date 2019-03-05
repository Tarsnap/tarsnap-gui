#ifndef HELPWIDGET_H
#define HELPWIDGET_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QDialog>
#include <QPlainTextEdit>
#include <QWidget>
WARNINGS_ENABLE

namespace Ui
{
class HelpWidget;
}

/*!
 * \ingroup widgets-main
 * \brief The HelpWidget is a QWidget which shows some documentation.
 */
class HelpWidget : public QWidget
{
    Q_OBJECT

#ifdef QT_TESTLIB_LIB
    friend class TestMainWindow;
    friend class TestSmallWidgets;
#endif

public:
    //! Constructor.
    explicit HelpWidget(QWidget *parent = nullptr);
    ~HelpWidget();

    //! Get a pointer to the console log.
    QPlainTextEdit *getConsoleLog();

public slots:
    //! The "about" app menu item was clicked.
    void aboutMenuClicked();

protected:
    //! Handles translation change of language.
    void changeEvent(QEvent *event);

private:
    Ui::HelpWidget *_ui;
    QString         _helpTabHTML;
    QDialog         _aboutWindow;
    QDialog         _consoleWindow;

    QPlainTextEdit *_consoleLog;

    void updateUi();
};

#endif // HELPWIDGET_H
