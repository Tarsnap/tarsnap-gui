#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QGridLayout>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QtTest/QtTest>
WARNINGS_ENABLE

#include "../qtest-platform.h"

#include "ElidedLabel.h"
#include "OkLabel.h"
#include "PathComboBrowse.h"
#include "PathLineBrowse.h"
#include "TWizard.h"
#include "TWizardPage.h"

class TestLibWidgets : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void elidedLabel();
    void elidedLabel_status();
    void okLabel();
    void pathlinebrowse();
    void pathcombobrowse();
    void twizard();
};

void TestLibWidgets::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    // Use a custom message handler to filter out unwanted messages
    IF_NOT_VISUAL { qInstallMessageHandler(offscreenMessageOutput); }
}

void TestLibWidgets::cleanupTestCase()
{
    // Wait up to 5 seconds to delete objects scheduled with ->deleteLater()
    QCoreApplication::processEvents(QEventLoop::AllEvents, 5000);
}

void TestLibWidgets::elidedLabel()
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

void TestLibWidgets::elidedLabel_status()
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

void TestLibWidgets::okLabel()
{
    OkLabel *ok = new OkLabel();

    VISUAL_INIT(ok);

    ok->setStatus(OkLabel::Ok);
    VISUAL_WAIT;

    ok->setStatus(OkLabel::Error);
    VISUAL_WAIT;

    ok->setStatus(OkLabel::Unset);
    VISUAL_WAIT;

    delete ok;
}

void TestLibWidgets::pathlinebrowse()
{
    PathLineBrowse *plb = new PathLineBrowse();
    QSignalSpy      sig_changed(plb, SIGNAL(textChanged(QString)));

    VISUAL_INIT(plb);
    IF_NOT_VISUAL { plb->show(); }

    plb->setLabel("label");
    QVERIFY(plb->label() == "label");
    VISUAL_WAIT;

    plb->setPlaceholderText("placeholder");
    QVERIFY(plb->placeholderText() == "placeholder");
    VISUAL_WAIT;

    plb->setDialogTitle("text");
    QVERIFY(plb->dialogTitle() == "text");
    VISUAL_WAIT;

    QVERIFY(sig_changed.count() == 0);
    plb->setText("text");
    QVERIFY(plb->text() == "text");
    QVERIFY(sig_changed.count() == 1);
    QVERIFY(sig_changed.takeFirst().at(0).value<QString>() == "text");
    sig_changed.clear();
    VISUAL_WAIT;

    plb->setStatusOk("");
    VISUAL_WAIT;

    plb->setStatusError("error message");
    VISUAL_WAIT;

    plb->setStatusOk("ok message");
    VISUAL_WAIT;

    plb->clear();
    QVERIFY(sig_changed.count() == 1);
    QVERIFY(sig_changed.takeFirst().at(0).value<QString>() == "");
    VISUAL_WAIT;

    // Test setText as a slot
    QLineEdit *le = new QLineEdit();
    connect(le, &QLineEdit::textChanged, plb, &PathLineBrowse::setText);

    plb->clear();
    le->setText("indirect text");
    QVERIFY(plb->text() == "indirect text");
    QVERIFY(sig_changed.count() == 1);
    QVERIFY(sig_changed.takeFirst().at(0).value<QString>() == "indirect text");
    VISUAL_WAIT;

    delete le;
    delete plb;
}

void TestLibWidgets::pathcombobrowse()
{
    PathComboBrowse *pcb = new PathComboBrowse();
    QSignalSpy       sig_changed(pcb, SIGNAL(textChanged(QString)));

    VISUAL_INIT(pcb);
    IF_NOT_VISUAL { pcb->show(); }

    pcb->setLabel("label");
    QVERIFY(pcb->label() == "label");
    VISUAL_WAIT;

    pcb->setPlaceholderText("placeholder");
    QVERIFY(pcb->placeholderText() == "placeholder");
    VISUAL_WAIT;

    pcb->setDialogTitle("text");
    QVERIFY(pcb->dialogTitle() == "text");
    VISUAL_WAIT;

    pcb->setDialogFilter("Text files (*.txt *.text)");
    QVERIFY(pcb->dialogFilter() == "Text files (*.txt *.text)");
    VISUAL_WAIT;

    QVERIFY(sig_changed.count() == 0);
    pcb->setText("text");
    QVERIFY(pcb->text() == "text");
    QVERIFY(sig_changed.count() == 1);
    QVERIFY(sig_changed.takeFirst().at(0).value<QString>() == "text");
    sig_changed.clear();
    VISUAL_WAIT;

    QVERIFY(pcb->count() == 1);
    pcb->addItem("another");
    QVERIFY(sig_changed.count() == 0);
    QVERIFY(pcb->count() == 2);
    VISUAL_WAIT;

    // Go back to the previous text
    pcb->setText("another");
    QVERIFY(sig_changed.count() == 1);
    pcb->setText("text");
    QVERIFY(sig_changed.count() == 2);
    VISUAL_WAIT;

    sig_changed.clear();
    pcb->setText("third");
    QVERIFY(pcb->text() == "third");
    QVERIFY(sig_changed.count() == 1);
    QVERIFY(sig_changed.takeFirst().at(0).value<QString>() == "third");
    VISUAL_WAIT;

    pcb->setStatusOk("");
    VISUAL_WAIT;

    pcb->setStatusError("error message");
    VISUAL_WAIT;

    pcb->setStatusOk("ok message");
    VISUAL_WAIT;

    sig_changed.clear();
    pcb->clear();
    QVERIFY(sig_changed.count() == 1);
    QVERIFY(sig_changed.takeFirst().at(0).value<QString>() == "");
    VISUAL_WAIT;

    // Test setText as a slot
    QLineEdit *le = new QLineEdit();
    connect(le, &QLineEdit::textChanged, pcb, &PathComboBrowse::setText);

    sig_changed.clear();
    pcb->clear();
    le->setText("indirect text");
    QVERIFY(pcb->text() == "indirect text");
    QVERIFY(sig_changed.count() == 1);
    QVERIFY(sig_changed.takeFirst().at(0).value<QString>() == "indirect text");
    VISUAL_WAIT;

    delete le;
    delete pcb;
}

void TestLibWidgets::twizard()
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

QTEST_MAIN(TestLibWidgets)
#include "test-lib-widgets.moc"
