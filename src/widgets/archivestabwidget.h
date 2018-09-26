#ifndef ARCHIVESTABWIDGET_H
#define ARCHIVESTABWIDGET_H

#include "ui_archivestabwidget.h"

#include <QEvent>
#include <QWidget>

/*!
 * \ingroup widgets-main
 * \brief The ArchivesTabWidget is a QWidget which shows the Archives tab.
 */
class ArchivesTabWidget : public QWidget
{
    Q_OBJECT

public:
    //! Constructor.
    explicit ArchivesTabWidget(QWidget *parent = nullptr);

protected:
    //! Handles translation change of language.
    void changeEvent(QEvent *event);

private slots:
    void commitSettings();

private:
    Ui::ArchivesTabWidget _ui;

    void updateUi();
    void loadSettings();
};

#endif // ARCHIVESTABWIDGET_H
