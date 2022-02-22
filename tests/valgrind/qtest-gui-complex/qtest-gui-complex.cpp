#include "../warnings-disable.h"

WARNINGS_DISABLE
#include <QtTest/QtTest>

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QFileSystemModel>
#include <QFontMetrics>
#include <QHostInfo>
#include <QImageReader>
#include <QLineEdit>
#include <QRadioButton>
#include <QSequentialAnimationGroup>
#include <QTextDocument>
#include <QTreeView>
#include <QVBoxLayout>
WARNINGS_ENABLE

class TestQTestComplex : public QObject
{
    Q_OBJECT

private slots:
    void pl_nothing();
    void pl_fontmetric_elidedText();
    void pl_fontmetric_size();
    void pl_processEvents();
    void pl_postEvent();
    void pl_filesystemmodel();
    void pl_button_checked();
    void pl_lineedit_text();
    void pl_lineedit_text_clear();
    void pl_lineedit_text_localhostname();
    void pl_lineedit_clear_show();
    void pl_checkbox_text();
    void pl_treeview();
    void pl_dialog_exec();
    void pl_combo_box();
    void pl_widget_show_hide();
    void pl_lineedit_placeholder();
    void pl_animation_start();
    void pl_image_canRead();
    void pl_textdocument();
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

void TestQTestComplex::pl_fontmetric_size()
{
    QFontMetrics metrics(qApp->font());
    metrics.size(Qt::TextSingleLine, "this is a medium piece of text");
}

void TestQTestComplex::pl_processEvents()
{
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void TestQTestComplex::pl_postEvent()
{
    QObject *obj   = new QObject();
    QEvent  *event = new QEvent(QEvent::FocusIn);

    QCoreApplication::postEvent(obj, event, 0);

    // Do not delete event; the event queue owns it now.
    delete obj;
}

void TestQTestComplex::pl_filesystemmodel()
{
    QFileSystemModel *model = new QFileSystemModel();

    // Start to read the path (this starts a new thread)
    model->setRootPath(QDir::currentPath());
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

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

void TestQTestComplex::pl_lineedit_text_clear()
{
    QLineEdit *le = new QLineEdit();

    le->setClearButtonEnabled(true);
    le->setText("Let's cause a memory leak!");
    le->clear();

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

void TestQTestComplex::pl_lineedit_clear_show()
{
    QLineEdit *le = new QLineEdit();

    le->show();
    le->setClearButtonEnabled(true);
    le->setPlaceholderText("placeholder");
    le->setText("text");
    delete le;
}

void TestQTestComplex::pl_checkbox_text()
{
    QCheckBox *cb = new QCheckBox();

    cb->setObjectName("cb");
    cb->setText("Let's cause a memory leak!");

    delete cb;
}

void TestQTestComplex::pl_treeview()
{
    QWidget     *widget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout();

    // Must set the layout before creating the QTreeView.
    widget->setLayout(layout);
    QTreeView *tv = new QTreeView(widget);
    delete tv;

    delete layout;
    delete widget;
}

class DiaExec : public QDialog
{
    Q_OBJECT
public:
    DiaExec(QWidget *parent = nullptr) : QDialog(parent) {}

    // This whole class is just so that we can wrap the exec() in a pl_*
    // function, so that our valgrind infrastructure can correctly generalize
    // the suppressions correctly.
    void pl_exec() { exec(); }
};

void TestQTestComplex::pl_dialog_exec()
{
    DiaExec *dia = new DiaExec();
    QMetaObject::invokeMethod(dia, "close", Qt::QueuedConnection);
    dia->pl_exec();
    delete dia;
}

void TestQTestComplex::pl_combo_box()
{
    QComboBox *box = new QComboBox();
    box->itemDelegate();
    delete box;
}

void TestQTestComplex::pl_widget_show_hide()
{
    QWidget *wid = new QWidget();
    wid->hide();
    wid->show();
    delete wid;
}

void TestQTestComplex::pl_lineedit_placeholder()
{
    QLineEdit *le = new QLineEdit();
    le->setPlaceholderText("this is a long piece of placeholder text");
    delete le;
}

void TestQTestComplex::pl_animation_start()
{
    // Somehow there's a leak caused by SetupWizard (even once BusyWidget is
    // completely removed) in QAbstractAnimation::start().  This appears to
    // reproduce the problem, although I have no clue how it applies to
    // the heavily-pared-down SetupWizard (introPage and cliPage only, and
    // no interaction on cliPage other than one QLineEdit::setText()).
    QSequentialAnimationGroup *a = new QSequentialAnimationGroup();
    a->start();
    delete a;
}

void TestQTestComplex::pl_image_canRead()
{
    QImageReader *ir = new QImageReader("../16x16-white.png");
    ir->canRead();
    delete ir;
}

void TestQTestComplex::pl_textdocument()
{
    QTextDocument *doc = new QTextDocument();
    doc->documentLayout();
    delete doc;
}

QTEST_MAIN(TestQTestComplex)
#include "qtest-gui-complex.moc"
