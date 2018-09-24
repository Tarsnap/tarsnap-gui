#ifndef HELPWIDGET_H
#define HELPWIDGET_H

#include "ui_helpwidget.h"

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

protected:
    //! Handles translation change of language.
    void changeEvent(QEvent *event);

private:
    Ui::HelpWidget _ui;

    void updateUi();
};

#endif // HELPWIDGET_H
