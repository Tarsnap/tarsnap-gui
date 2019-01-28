#include <QtTest/QtTest>

#include "../qtest-platform.h"

#include "confirmationdialog.h"
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
    void filepickerwidget();
    void filepickerdialog();
    void confirmationDialog();
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

QTEST_MAIN(TestSmallWidgets)
#include "test-small-widgets.moc"
