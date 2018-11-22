#include <QtTest/QtTest>

#include <QFileSystemModel>
#include <QFontMetrics>
#include <QHostInfo>
#include <QLineEdit>
#include <QRadioButton>

class TestQTestComplex : public QObject
{
    Q_OBJECT

private slots:
    void pl_nothing();
    void pl_fontmetric_elidedText();
    void pl_processEvents();
    void pl_filesystemmodel();
    void pl_button_checked();
    void pl_lineedit_text();
    void pl_lineedit_text_localhostname();
};

void TestQTestComplex::pl_nothing()
{
}

void TestQTestComplex::pl_fontmetric_elidedText()
{
    QFontMetrics metrics(qApp->font());
    QString text = metrics.elidedText("this is a long long long piece of text",
                                      Qt::ElideRight, 100);
}

void TestQTestComplex::pl_processEvents()
{
    QCoreApplication::processEvents(0, 100);
}

void TestQTestComplex::pl_filesystemmodel()
{
    QFileSystemModel *model = new QFileSystemModel();

    // Start to read the path (this starts a new thread)
    model->setRootPath(QDir::currentPath());
    QCoreApplication::processEvents(0, 100);

    delete model;
}

void TestQTestComplex::pl_button_checked()
{
    QRadioButton *button = new QRadioButton();

    button->setChecked(true);

    delete button;
}

void TestQTestComplex::pl_lineedit_text()
{
    QLineEdit *le = new QLineEdit();

    le->setClearButtonEnabled(true);
    le->setText("Let's cause a memory leak!");

    delete le;
}

void TestQTestComplex::pl_lineedit_text_localhostname()
{
    // Prepare for the memory leak.  All these lines are necessary if this
    // test is being run independently.  If this test is being run immediately
    // after pl_lineedit_text() then the QHostInfo::localHostName() shows up
    // as causing a leak in pl_lineedit_text().
    QLineEdit *le = new QLineEdit();
    le->setClearButtonEnabled(true);
    le->setText("Let's cause a memory leak with localHostName!");
    delete le;

    // This line causes a memory leak to appear from QLineEdit::textChanged()
    // on Ubuntu 16.04 with Qt 5.11.1.  (Maybe other platforms as well.)
    //
    // Seriously.  No, I wouldn't believe it either if I hadn't seen it myself
    // multiple times.
    QHostInfo::localHostName();
}

QTEST_MAIN(TestQTestComplex)
#include "qtest-gui-complex.moc"
