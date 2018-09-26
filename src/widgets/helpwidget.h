#ifndef HELPWIDGET_H
#define HELPWIDGET_H

#include "ui_helpwidget.h"

#include <QPlainTextEdit>
#include <QWidget>

/*!
 * \ingroup widgets-main
 * \brief The HelpWidget is a QWidget which shows some documentation.
 */
class HelpWidget : public QWidget
{
    Q_OBJECT

#ifdef QT_TESTLIB_LIB
    friend class TestMainWindow;
#endif

public:
    //! Constructor.
    explicit HelpWidget(QWidget *parent = nullptr);

    //! Get a pointer to the console log.
    QPlainTextEdit *getConsoleLog();

public slots:
    //! The "about" app menu item was clicked.
    void aboutMenuClicked();

protected:
    //! Handles translation change of language.
    void changeEvent(QEvent *event);

private:
    Ui::HelpWidget _ui;
    QString        _helpTabHTML;
    QDialog        _aboutWindow;
    QDialog        _consoleWindow;

    QPlainTextEdit *_consoleLog;

    void updateUi();
};

#endif // HELPWIDGET_H
