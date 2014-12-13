#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QLabel>

namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();

protected:
    void paintEvent(QPaintEvent *);

private:
    Ui::MainWidget *ui;
    QLabel *m_tarsnapLogo;
};

#endif // MAINWIDGET_H
