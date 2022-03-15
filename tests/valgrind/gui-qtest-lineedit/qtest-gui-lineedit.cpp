#include "../warnings-disable.h"

WARNINGS_DISABLE
#include <QtTest/QtTest>

#include <QLineEdit>
WARNINGS_ENABLE

class TestQTestLineEdit : public QObject
{
    Q_OBJECT

private slots:
    void pl_nothing();
    void pl_lineedit_text();
    void pl_lineedit_text_show();
    void pl_lineedit_text_clear();
    void pl_lineedit_text_clear_show();
    void pl_lineedit_style_clear();
    void pl_lineedit_style_clear_show();
    void pl_lineedit_clear_show();
    void pl_lineedit_placeholder();
    void pl_lineedit_placeholder_show();
};

void TestQTestLineEdit::pl_nothing()
{
}

void TestQTestLineEdit::pl_lineedit_text()
{
    QLineEdit *le = new QLineEdit();

    le->setClearButtonEnabled(true);
    le->setText("Let's cause a memory leak!");

    delete le;
}

void TestQTestLineEdit::pl_lineedit_text_show()
{
    QLineEdit *le = new QLineEdit();
    le->setClearButtonEnabled(true);
    le->show();
    le->setText("text");
    delete le;
}

void TestQTestLineEdit::pl_lineedit_text_clear()
{
    QLineEdit *le = new QLineEdit();

    le->setClearButtonEnabled(true);
    le->setText("Let's cause a memory leak!");
    le->clear();

    delete le;
}

void TestQTestLineEdit::pl_lineedit_text_clear_show()
{
    QLineEdit *le = new QLineEdit();

    le->show();
    le->setClearButtonEnabled(true);
    le->setText("Let's cause a memory leak!");
    le->clear();

    delete le;
}

void TestQTestLineEdit::pl_lineedit_style_clear()
{
    QLineEdit *le = new QLineEdit();
    le->setStyleSheet("padding-left: 3px;");
    le->setClearButtonEnabled(true);
    delete le;
}

void TestQTestLineEdit::pl_lineedit_style_clear_show()
{
    QLineEdit *le = new QLineEdit();
    le->show();
    le->setStyleSheet("padding-left: 3px;");
    le->setClearButtonEnabled(true);
    le->setText("text");
    delete le;
}

void TestQTestLineEdit::pl_lineedit_clear_show()
{
    QLineEdit *le = new QLineEdit();

    le->show();
    le->setClearButtonEnabled(true);
    le->setPlaceholderText("placeholder");
    le->setText("text");
    delete le;
}

void TestQTestLineEdit::pl_lineedit_placeholder()
{
    QLineEdit *le = new QLineEdit();
    le->setPlaceholderText("this is a long piece of placeholder text");
    delete le;
}

void TestQTestLineEdit::pl_lineedit_placeholder_show()
{
    QLineEdit *le = new QLineEdit();
    le->show();
    le->setPlaceholderText("this is a long piece of placeholder text");
    delete le;
}

QTEST_MAIN(TestQTestLineEdit)
#include "qtest-gui-lineedit.moc"
