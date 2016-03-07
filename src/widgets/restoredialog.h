#ifndef RESTOREDIALOG_H
#define RESTOREDIALOG_H

#include "persistentmodel/archive.h"

#include <QDialog>

namespace Ui
{
class RestoreDialog;
}

class RestoreDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RestoreDialog(ArchivePtr archive, QWidget *parent = 0);
    ~RestoreDialog();

    ArchiveRestoreOptions getOptions();

private slots:
    void baseDirToggled(bool checked);
    void changeDir();
    void changeArchive();
    bool validate();
    void getArchiveToggled(bool checked);
    void preservePathsToggled(bool checked);

private:
    Ui::RestoreDialog *_ui;
    ArchivePtr         _archive;
    QString            _downDir;
};

#endif // RESTOREDIALOG_H
