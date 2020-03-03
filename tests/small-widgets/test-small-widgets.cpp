#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QPixmap>
#include <QPushButton>
#include <QtTest/QtTest>
WARNINGS_ENABLE

#include "../qtest-platform.h"

#include "confirmationdialog.h"
#include "elidedannotatedlabel.h"
#include "filepickerdialog.h"
#include "filepickerwidget.h"

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
    QCoreApplication::processEvents(QEventLoop::AllEvents, 5000);
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
    QWidget *         widget = new QWidget();
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
    cd->_inputDialog.close();
    VISUAL_WAIT;
    QVERIFY((sig_confirmed.count() == 0) && (sig_cancel.count() == 1));
    sig_cancel.clear();

    // Launch the window and accept it (with blank text).
    cd->start("text title", "type 'confirm'", "confirm", 1, "countdown title",
              "seconds left %1", "confirmed");
    VISUAL_WAIT;
    cd->_inputDialog.accept();
    VISUAL_WAIT;
    QVERIFY((sig_confirmed.count() == 0) && (sig_cancel.count() == 1));
    sig_cancel.clear();

    // Launch the window and accept it (with incorrect text).
    cd->start("text title", "type 'confirm'", "confirm", 1, "countdown title",
              "seconds left %1", "confirmed");
    VISUAL_WAIT;
    cd->_inputDialog.setTextValue("some kind of text");
    VISUAL_WAIT;
    cd->_inputDialog.accept();
    VISUAL_WAIT;
    QVERIFY((sig_confirmed.count() == 0) && (sig_cancel.count() == 1));
    sig_cancel.clear();

    // Launch the window and accept it (with too-long text).
    cd->start("text title", "type 'confirm'", "confirm", 1, "countdown title",
              "seconds left %1", "confirmed");
    QVERIFY(cd->_inputDialog.textValue() == "");
    VISUAL_WAIT;
    cd->_inputDialog.setTextValue("confirmnot");
    VISUAL_WAIT;
    cd->_inputDialog.accept();
    VISUAL_WAIT;
    QVERIFY((sig_confirmed.count() == 0) && (sig_cancel.count() == 1));
    sig_cancel.clear();

    // Launch the window, enter 'confirm', then cancel the countdown.
    cd->start("text title", "type 'confirm'", "confirm", 1, "countdown title",
              "seconds left %1", "confirmed");
    VISUAL_WAIT;
    cd->_inputDialog.setTextValue("confirm");
    VISUAL_WAIT;
    cd->_inputDialog.accept();
    VISUAL_WAIT;
    cd->_countdownBox.reject();
    QVERIFY((sig_confirmed.count() == 0) && (sig_cancel.count() == 1));
    sig_cancel.clear();

    // Launch the window, enter 'confirm', then wait for the countdown.
    cd->start("text title", "type 'confirm'", "confirm", 1, "countdown title",
              "seconds left %1", "confirmed");
    QVERIFY(cd->_inputDialog.textValue() == "");
    VISUAL_WAIT;
    cd->_inputDialog.setTextValue("confirm");
    VISUAL_WAIT;
    cd->_inputDialog.accept();
    VISUAL_WAIT;
    while(sig_confirmed.count() == 0)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    QVERIFY((sig_confirmed.count() == 1) && (sig_cancel.count() == 0));
    sig_confirmed.clear();

    delete cd;
}

QTEST_MAIN(TestSmallWidgets)
#include "test-small-widgets.moc"
