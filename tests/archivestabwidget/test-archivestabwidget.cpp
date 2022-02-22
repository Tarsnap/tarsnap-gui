#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QCoreApplication>
#include <QDateTime>
#include <QMetaType>
#include <QModelIndex>
#include <QObject>
#include <QSignalSpy>
#include <QString>
#include <QTest>
#include <QThreadPool>
#include <QVariant>
#include <QVector>

#include "ui_archivestabwidget.h"
WARNINGS_ENABLE

#include "../qtest-platform.h"

#include "messages/archivefilestat.h"

#include "basetask.h"
#include "filetablemodel.h"
#include "persistentmodel/archive.h"
#include "widgets/archivelistwidget.h"
#include "widgets/archivestabwidget.h"
#include "widgets/archivewidget.h"

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
    qRegisterMetaType<QVector<FileStat>>("QVector<FileStat>");
    qRegisterMetaType<BaseTask *>("BaseTask *");
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
    ArchivesTabWidget     *archivestabwidget = new ArchivesTabWidget();
    Ui::ArchivesTabWidget *ui                = archivestabwidget->_ui;
    ArchiveListWidget     *alw               = ui->archiveListWidget;

    FileTableModel *fm = ui->archiveDetailsWidget->_contentsModel;

    VISUAL_INIT(archivestabwidget);
    VISUAL_WAIT;

    Archive   *actual_archive = new Archive();
    ArchivePtr archive(actual_archive);
    archive->setName("archive1");
    archive->setContents("-rw-r--r-- 0 user group 1234 Jan 1 2019 myfile");

    alw->addArchive(archive);
    VISUAL_WAIT;

    QSignalSpy sig_taskRequest(archivestabwidget,
                               SIGNAL(taskRequested(BaseTask *)));
    archivestabwidget->displayInspectArchive(archive);
    VISUAL_WAIT;

    // Wait for archive parsing signal.
    WAIT_SIG(sig_taskRequest);

    // Get the provided test
    BaseTask *task = sig_taskRequest.takeFirst().at(0).value<BaseTask *>();

    // Fake a reply.
    FileStat parsed = {"myfile", "Jan 1 2019", 1234, "user", "group", "644", 0};
    QVector<FileStat> files = {parsed};
    fm->setFiles(files);
    VISUAL_WAIT;

    // Check that we have 1 file, with 7 pieces of info.
    QVERIFY(fm->rowCount() == 1);
    QVERIFY(fm->columnCount() == 7);

    // Check the filename
    QModelIndex index    = fm->index(0, 0);
    QString     filename = fm->data(index).toString();
    QVERIFY(filename == "myfile");

    // Clean up
    delete task;
    delete archivestabwidget;
}

QTEST_MAIN(TestArchivesTabWidget)
WARNINGS_DISABLE
#include "test-archivestabwidget.moc"
WARNINGS_ENABLE
