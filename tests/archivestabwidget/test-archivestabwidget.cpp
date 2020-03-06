#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QThreadPool>
#include <QtTest/QtTest>

#include "ui_archivestabwidget.h"
WARNINGS_ENABLE

#include "../qtest-platform.h"

#include "archivelistwidget.h"
#include "archivestabwidget.h"

#include "TSettings.h"

class TestArchivesTabWidget : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void archiveListWidget();
    void displayArchive();
};

void TestArchivesTabWidget::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    // Use a custom message handler to filter out unwanted messages
    IF_NOT_VISUAL { qInstallMessageHandler(offscreenMessageOutput); }

    // Initialization normally done in init_shared.cpp's init_no_app()
    qRegisterMetaType<QVector<File>>("QVector<File>");
}

void TestArchivesTabWidget::cleanupTestCase()
{
    TSettings::destroy();

    // Wait up to 5 seconds for any running threads to stop.
    QThreadPool::globalInstance()->waitForDone(5000);
    // Wait up to 5 seconds to delete objects scheduled with ->deleteLater()
    WAIT_FINAL;
}

void TestArchivesTabWidget::archiveListWidget()
{
    ArchiveListWidget *alw = new ArchiveListWidget();

    VISUAL_INIT(alw);

    ArchivePtr archive(new Archive);
    archive->setName("archive1 pXq");
    archive->setSizeTotal(123000);
    archive->setTimestamp(QDateTime::currentDateTime());

    ArchivePtr archive2(new Archive);
    archive2->setName("backup2");
    archive2->setSizeTotal(4000);
    archive2->setTimestamp(QDateTime::currentDateTime());

    ArchivePtr archive3(new Archive);
    archive3->setName("Job_longname_2019-01-01_02-03-04");
    archive3->setSizeTotal(56);
    archive3->setTimestamp(QDateTime::currentDateTime());
    archive3->setJobRef("logname");

    alw->addArchive(archive);
    VISUAL_WAIT;

    alw->addArchive(archive2);
    VISUAL_WAIT;

    alw->addArchive(archive3);
    VISUAL_WAIT;

    alw->selectArchive(archive);
    VISUAL_WAIT;

    alw->setFilter("back");
    VISUAL_WAIT;

    alw->inspectArchive(archive2);
    VISUAL_WAIT;

    delete alw;
}

void TestArchivesTabWidget::displayArchive()
{
    ArchivesTabWidget *    archivestabwidget = new ArchivesTabWidget();
    Ui::ArchivesTabWidget *ui                = archivestabwidget->_ui;
    ArchiveListWidget *    alw               = ui->archiveListWidget;

    VISUAL_INIT(archivestabwidget);
    VISUAL_WAIT;

    Archive *  actual_archive = new Archive();
    ArchivePtr archive(actual_archive);
    archive->setName("archive1");

    alw->addArchive(archive);
    VISUAL_WAIT;

    QSignalSpy sig_fileList(actual_archive, SIGNAL(fileList(QVector<File>)));
    archivestabwidget->displayInspectArchive(archive);
    VISUAL_WAIT;

    // Wait for archive parsing to finish
    while(sig_fileList.count() == 0)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

    delete archivestabwidget;
}

QTEST_MAIN(TestArchivesTabWidget)
#include "test-archivestabwidget.moc"
