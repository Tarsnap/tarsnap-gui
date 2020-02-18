#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QPixmap>
#include <QPushButton>
#include <QtTest/QtTest>
WARNINGS_ENABLE

#include "../qtest-platform.h"

#include "busywidget.h"
#include "confirmationdialog.h"
#include "elidedannotatedlabel.h"
#include "filepickerdialog.h"
#include "filepickerwidget.h"

#include "ElidedLabel.h"
#include "TSettings.h"
#include "TWizard.h"
#include "TWizardPage.h"

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
    void busywidget_on_off();
    void statuslabel();
    void twizard();
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
    IF_NOT_VISUAL { bw->show(); }

    delete bw;
}

void TestSmallWidgets::busywidget_on_off()
{
    BusyWidget *bw = new BusyWidget();

    VISUAL_INIT(bw);
    IF_NOT_VISUAL { bw->show(); }

    // Before starting
    VISUAL_WAIT;

    // Animation
    bw->animate(true);
    QTest::qWait(100);
    VISUAL_WAIT;

    // After animation
    bw->animate(false);
    QTest::qWait(100);
    VISUAL_WAIT;

    delete bw;
}

void TestSmallWidgets::statuslabel()
{
    ElidedLabel *el = new ElidedLabel();
    el->setMinimumWidth(200);

    VISUAL_INIT(el);

    el->messageNormal("normal message");
    VISUAL_WAIT;

    el->messageError("error message");
    VISUAL_WAIT;

    delete el;
}

void TestSmallWidgets::twizard()
{
    TWizard *wizard = new TWizard();

    // Add logo
    QPixmap pix(32, 32);
    pix.fill(Qt::blue);
    wizard->setLogo(pix);

    // Create pages with titles
    TWizardPage *p1 = new TWizardPage();
    TWizardPage *p2 = new TWizardPage();
    p1->setTitle("page 1");
    p2->setTitle("page 2");

    // Add a "proceed" button to each page
    QPushButton *pb1 = new QPushButton(p1);
    pb1->setText("Next");
    pb1->setObjectName("nextButton");
    QPushButton *pb2 = new QPushButton(p2);
    pb2->setText("Finish");
    pb2->setObjectName("finishButton");

    // Add pages to the wizard; must be after the buttons have been added
    wizard->addPages(QList<TWizardPage *>() << p1 << p2);

    // Navigate through the pages.
    VISUAL_INIT(wizard);
    IF_NOT_VISUAL { wizard->open(); }

    wizard->currentPage()->button(TWizardPage::NextButton)->click();
    VISUAL_WAIT;
    wizard->currentPage()->button(TWizardPage::FinishButton)->click();
    VISUAL_WAIT;

    // We should have quit the wizard
    QVERIFY(wizard->isVisible() == false);

    delete wizard;
}

QTEST_MAIN(TestSmallWidgets)
#include "test-small-widgets.moc"
