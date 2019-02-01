#include <QtTest/QtTest>

#include "../qtest-platform.h"

#include "archivelistwidget.h"
#include "archivestabwidget.h"

#include "ui_archivestabwidget.h"

#include "TSettings.h"

class TestArchivesTabWidget : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void archivesListWidget();
    void displayArchive();
};

void TestArchivesTabWidget::initTestCase()
{
    QCoreApplication::setOrganizationName(TEST_NAME);

    IF_NOT_VISUAL
    {
        // Use a custom message handler to filter out unwanted messages
        orig_message_handler = qInstallMessageHandler(offscreenMessageOutput);
    }

    // Initialization normally done in init_shared.cpp's init_no_app()
    qRegisterMetaType<QVector<File>>("QVector<File>");
}

void TestArchivesTabWidget::cleanupTestCase()
{
    TSettings::destroy();
}

void TestArchivesTabWidget::archivesListWidget()
{
    ArchiveListWidget *alw = new ArchiveListWidget();

    VISUAL_INIT(alw);

    ArchivePtr archive(new Archive);
    archive->setName("archive1");
    ArchivePtr archive2(new Archive);
    archive->setName("backup2");

    alw->addArchive(archive);
    VISUAL_WAIT;

    alw->addArchive(archive2);
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

    ArchivePtr archive(new Archive);
    archive->setName("archive1");

    alw->addArchive(archive);
    VISUAL_WAIT;

    archivestabwidget->displayInspectArchive(archive);
    VISUAL_WAIT;

    delete archivestabwidget;
}

QTEST_MAIN(TestArchivesTabWidget)
#include "test-archivestabwidget.moc"
