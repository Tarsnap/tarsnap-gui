#include <QtTest/QtTest>

#include "../qtest-platform.h"

#include "busywidget.h"
#include "confirmationdialog.h"
#include "elidedannotatedlabel.h"
#include "elidedlabel.h"
#include "filepickerdialog.h"
#include "filepickerwidget.h"

#include "TSettings.h"

class TestSmallWidgets : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void elidedLabel();
    void elidedAnnotatedLabel();
    void filepickerwidget();
    void filepickerdialog();
    void confirmationDialog();
    void busywidget();
};

void TestSmallWidgets::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    IF_NOT_VISUAL
    {
        // Use a custom message handler to filter out unwanted messages
        orig_message_handler = qInstallMessageHandler(offscreenMessageOutput);
    }
}

void TestSmallWidgets::cleanupTestCase()
{
    TSettings::destroy();
}

void TestSmallWidgets::elidedLabel()
{
    // Initialize variables.
    const QString text("this is a long long long piece of text");
    ElidedLabel * elidedlabel = new ElidedLabel();

    VISUAL_INIT(elidedlabel);

    // Set up ElidedText.
    elidedlabel->setElide(Qt::ElideRight);
    elidedlabel->setText(text);

    // The text should be fully visible if there's enough room.
    elidedlabel->setGeometry(100, 100, 1000, 20);
    QVERIFY(elidedlabel->text() == text);
    QVERIFY(elidedlabel->elideText(text).count() == text.count());
    VISUAL_WAIT;

    // The text should be elided if the widget is too narrow.
    elidedlabel->setGeometry(100, 100, 100, 20);
    QVERIFY(elidedlabel->text() == text);
    QVERIFY(elidedlabel->elideText(text).count() < text.count());
    VISUAL_WAIT;

    delete elidedlabel;
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
    ConfirmationDialog *confirm = new ConfirmationDialog();
    delete confirm;
}

void TestSmallWidgets::busywidget()
{
    BusyWidget *bw = new BusyWidget();

    VISUAL_INIT(bw);

    bw->animate();
    QTest::qWait(100);
    VISUAL_WAIT;

    bw->stop();
    QTest::qWait(100);
    VISUAL_WAIT;

    delete bw;
}

QTEST_MAIN(TestSmallWidgets)
#include "test-small-widgets.moc"
