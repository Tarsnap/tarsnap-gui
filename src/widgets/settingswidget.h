#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include "scheduling.h"
#include "tarsnapaccount.h"
#include "taskmanager.h"
#include "ui_settingswidget.h"

#include <QMenuBar>
#include <QMessageBox>
#include <QTimer>
#include <QWidget>

/*!
 * \ingroup widgets-main
 * \brief The SettingsWidget is a QWidget which is handles configuration.
 */
class SettingsWidget : public QWidget
{
    Q_OBJECT

#ifdef QT_TESTLIB_LIB
    friend class TestSettingsWidget;
#endif

public:
    //! Constructor.
    explicit SettingsWidget(QWidget *parent = nullptr);

public slots:
    //! Initialization routines.
    void initializeSettingsWidget();

signals:

private slots:
    void commitSettings();

private:
    Ui::SettingsWidget _ui;

    void updateUi();
    void loadSettings();
};

#endif // SETTINGSWIDGET_H
