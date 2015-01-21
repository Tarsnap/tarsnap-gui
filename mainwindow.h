#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "jobmanager.h"

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

signals:
    void backupNow(BackupJob job);

protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent * event);

private slots:
    void updateBackupItemTotals(qint64 count, qint64 size);

    void on_appendTimestampCheckBox_toggled(bool checked);
    void on_backupListInfoLabel_linkActivated(const QString &link);

    void on_backupButton_clicked();

private:
    Ui::MainWindow *_ui;
    QLabel *_tarsnapLogo;
    QPoint  _windowDragPos;
    QString _lastTimestamp;
};

#endif // MAINWINDOW_H
