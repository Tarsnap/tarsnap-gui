#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QList>
#include <QMessageBox>
#include <QObject>
#include <QPushButton>
#include <QSignalSpy>
#include <QString>
#include <QTest>
#include <QUrl>
#include <QVariant>
#include <QVector>
#include <QWidget>
#include <Qt>

#include "ui_stoptasksdialog.h"
WARNINGS_ENABLE

#include "../qtest-platform.h"

#include "widgets/confirmationdialog.h"
#include "widgets/elidedannotatedlabel.h"
#include "widgets/filepickerdialog.h"
#include "widgets/filepickerwidget.h"
#include "widgets/stoptasksdialog.h"

#include "TSettings.h"

class TestSmallWidgets : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void elidedAnnotatedLabel();
    void filepickerwidget();
    void filepickerdialog();
    void confirmationDialog();
    void stoptasksdialog();
    void stoptasksdialog_showall();
    void stoptasksdialog_update();
};

void TestSmallWidgets::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    // Use a custom message handler to filter out unwanted messages
    IF_NOT_VISUAL { qInstallMessageHandler(offscreenMessageOutput); }
}

void TestSmallWidgets::cleanupTestCase()
{
    TSettings::destroy();

    // Wait up to 5 seconds to delete objects scheduled with ->deleteLater()
    WAIT_FINAL;
}

void TestSmallWidgets::elidedAnnotatedLabel()
{
    ElidedAnnotatedLabel *el = new ElidedAnnotatedLabel();

    const QString plain("grey black long piece of text again grey");
    const QString full("<font color=\"grey\">grey</font> black long piece of "
                       "text <font color=\"grey\">again grey</font>");
    QVector<QString> texts(3);
    texts[0] = "grey";
    texts[1] = " black long piece of text ";
    texts[2] = "again grey";
    QVector<QString> annotations(6, "");
    annotations[0] = "<font color=\"grey\">";
    annotations[1] = "</font>";
    annotations[4] = "<font color=\"grey\">";
    annotations[5] = "</font>";

    VISUAL_INIT(el);

    // Set up ElidedText.
    el->setElide(Qt::ElideRight);
    el->setAnnotatedText(texts, annotations);

    // The text should be fully visible if there's enough room.
    el->setGeometry(100, 100, 1000, 20);
    QVERIFY(el->text() == plain);
    QVERIFY(el->elideText().count() == full.count());
    VISUAL_WAIT;

    // The text should be elided if the widget is too narrow.
    el->setGeometry(100, 100, 100, 20);
    QVERIFY(el->text() == plain);
    QVERIFY(el->elideText().count() < full.count());
    VISUAL_WAIT;

    delete el;
}

void TestSmallWidgets::filepickerwidget()
{
    FilePickerWidget *fpw = new FilePickerWidget();

    fpw->setSettingShowSystem(true);
    QVERIFY(fpw->settingShowSystem() == true);

    fpw->setSettingHideSymlinks(true);
    QVERIFY(fpw->settingHideSymlinks() == true);

    const QString filename = QString(TEST_DIR) + "/" + "empty-file";
    fpw->setCurrentPath(filename);
    QVERIFY(fpw->getCurrentPath() == filename);

    delete fpw;
}

void TestSmallWidgets::filepickerdialog()
{
    QWidget          *widget = new QWidget();
    FilePickerDialog *fpd    = new FilePickerDialog(widget);

    QList<QUrl> myurls({QUrl("file:///tmp")});
    fpd->setSelectedUrls(myurls);
    QVERIFY(fpd->getSelectedUrls() == myurls);

    delete fpd;
    delete widget;
}

void TestSmallWidgets::confirmationDialog()
{
    ConfirmationDialog *cd = new ConfirmationDialog();
    QSignalSpy          sig_cancel(cd, SIGNAL(cancelled()));
    QSignalSpy          sig_confirmed(cd, SIGNAL(confirmed()));

    // Don't try to ->show() this, because ConfirmationDialog is a
    // QObject (which spawns windows as needed).

    // Launch the window then close it.
    cd->start("text title", "type 'confirm'", "confirm", 1, "countdown title",
              "seconds left %1", "confirmed");
    VISUAL_WAIT;
    cd->_inputDialog->close();
    VISUAL_WAIT;
    QVERIFY((sig_confirmed.count() == 0) && (sig_cancel.count() == 1));
    sig_cancel.clear();

    // Launch the window and accept it (with blank text).
    cd->start("text title", "type 'confirm'", "confirm", 1, "countdown title",
              "seconds left %1", "confirmed");
    VISUAL_WAIT;
    cd->_inputDialog->accept();
    VISUAL_WAIT;
    QVERIFY((sig_confirmed.count() == 0) && (sig_cancel.count() == 1));
    sig_cancel.clear();

    // Launch the window and accept it (with incorrect text).
    cd->start("text title", "type 'confirm'", "confirm", 1, "countdown title",
              "seconds left %1", "confirmed");
    VISUAL_WAIT;
    cd->_inputDialog->setTextValue("some kind of text");
    VISUAL_WAIT;
    cd->_inputDialog->accept();
    VISUAL_WAIT;
    QVERIFY((sig_confirmed.count() == 0) && (sig_cancel.count() == 1));
    sig_cancel.clear();

    // Launch the window and accept it (with too-long text).
    cd->start("text title", "type 'confirm'", "confirm", 1, "countdown title",
              "seconds left %1", "confirmed");
    QVERIFY(cd->_inputDialog->textValue() == "");
    VISUAL_WAIT;
    cd->_inputDialog->setTextValue("confirmnot");
    VISUAL_WAIT;
    cd->_inputDialog->accept();
    VISUAL_WAIT;
    QVERIFY((sig_confirmed.count() == 0) && (sig_cancel.count() == 1));
    sig_cancel.clear();

    // Launch the window, enter 'confirm', then cancel the countdown.
    cd->start("text title", "type 'confirm'", "confirm", 1, "countdown title",
              "seconds left %1", "confirmed");
    VISUAL_WAIT;
    cd->_inputDialog->setTextValue("confirm");
    VISUAL_WAIT;
    cd->_inputDialog->accept();
    VISUAL_WAIT;
    cd->_countdownBox->reject();
    QVERIFY((sig_confirmed.count() == 0) && (sig_cancel.count() == 1));
    sig_cancel.clear();

    // Launch the window, enter 'confirm', then wait for the countdown.
    cd->start("text title", "type 'confirm'", "confirm", 1, "countdown title",
              "seconds left %1", "confirmed");
    QVERIFY(cd->_inputDialog->textValue() == "");
    VISUAL_WAIT;
    cd->_inputDialog->setTextValue("confirm");
    VISUAL_WAIT;
    cd->_inputDialog->accept();
    VISUAL_WAIT;
    WAIT_SIG(sig_confirmed);
    QVERIFY((sig_confirmed.count() == 1) && (sig_cancel.count() == 0));
    sig_confirmed.clear();

    delete cd;
}

void TestSmallWidgets::stoptasksdialog()
{
    StopTasksDialog *sd = new StopTasksDialog();
    QSignalSpy       sig_cancelAboutToQuit(sd, SIGNAL(cancelAboutToQuit()));
    QSignalSpy       sig_quitOk(sd, SIGNAL(quitOk()));
    QSignalSpy       sig_stop(sd, SIGNAL(stopTasks(bool, bool, bool)));
    QList<QVariant>  stop_bools;

    Ui::StopTasksDialog *ui = sd->_ui;

    // Don't VISUAL_INIT this one, because it's done internally.

    // Query with 1 running backup task, reject it.
    sd->display(true, 1, 0, true);
    VISUAL_WAIT;
    ui->buttonBox->button(QDialogButtonBox::Cancel)->clicked();
    QVERIFY(sd->isVisible() == false);
    QVERIFY(sig_cancelAboutToQuit.count() == 1);
    VISUAL_WAIT;

    // Query with 1 running backup task, stop it.
    sd->display(true, 1, 0, true);
    VISUAL_WAIT;
    ui->interruptButton->clicked();
    QVERIFY(sd->isVisible() == false);
    QVERIFY(sig_quitOk.count() == 1);
    QVERIFY(sig_stop.count() == 1);
    stop_bools = sig_stop.takeFirst();
    QVERIFY(stop_bools.size() == 3);
    QVERIFY(stop_bools.at(0).toBool() == true);
    QVERIFY(stop_bools.at(1).toBool() == false);
    QVERIFY(stop_bools.at(2).toBool() == true);

    VISUAL_WAIT;

    delete sd;
}

void TestSmallWidgets::stoptasksdialog_showall()
{
    StopTasksDialog *sd = new StopTasksDialog();

    // Don't VISUAL_INIT this one, because it's done internally.

    // Show "going to quit".
    sd->display(true, 1, 1, true);
    VISUAL_WAIT;
    sd->reject();

    // Show "not quitting".
    sd->display(true, 1, 1, false);
    VISUAL_WAIT;
    sd->reject();

    delete sd;
}

void TestSmallWidgets::stoptasksdialog_update()
{
    StopTasksDialog *sd = new StopTasksDialog();
    QSignalSpy       sig_cancelAboutToQuit(sd, SIGNAL(cancelAboutToQuit()));
    QSignalSpy       sig_quitOk(sd, SIGNAL(quitOk()));
    QSignalSpy       sig_stop(sd, SIGNAL(stopTasks(bool, bool, bool)));
    QList<QVariant>  stop_bools;

    // Don't VISUAL_INIT this one, because it's done internally.

    // Query all "quit" cases.
    sd->display(true, 1, 1, true);
    VISUAL_WAIT;

    sd->updateTasks(false, 1, 1);
    VISUAL_WAIT;

    sd->updateTasks(false, 0, 1);
    VISUAL_WAIT;

    sd->updateTasks(false, 1, 0);
    VISUAL_WAIT;

    // This command automatically sends an accept() to the dialog.
    sd->updateTasks(false, 0, 0);
    VISUAL_WAIT;

    // Query all "non-quit" cases.
    sd->display(true, 1, 1, false);
    VISUAL_WAIT;

    sd->updateTasks(false, 1, 1);
    VISUAL_WAIT;

    sd->updateTasks(false, 0, 1);
    VISUAL_WAIT;

    sd->updateTasks(false, 1, 0);
    VISUAL_WAIT;

    // This command automatically sends an accept() to the dialog.
    sd->updateTasks(false, 0, 0);
    VISUAL_WAIT;

    delete sd;
}

QTEST_MAIN(TestSmallWidgets)
WARNINGS_DISABLE
#include "test-small-widgets.moc"
WARNINGS_ENABLE
