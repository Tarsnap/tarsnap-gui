#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QLabel>

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void readSettings();

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent * event);

private slots:
    void on_appendTimestampCheckBox_toggled(bool checked);

    void on_backupListInfoLabel_linkActivated(const QString &link);

private:
    Ui::MainWindow *_ui;
    QLabel *_tarsnapLogo;
    QPoint  _windowDragPos;
};

#endif // MAINWINDOW_H
