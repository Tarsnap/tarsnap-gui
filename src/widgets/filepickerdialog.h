#ifndef FILEPICKERDIALOG_H
#define FILEPICKERDIALOG_H

#include "ui_filepickerdialog.h"

#include <QDialog>
#include <QList>
#include <QUrl>

/*!
 * \ingroup widgets-specialized
 * \brief The FilePickerDialog is a QDialog which provides a thin wrapper
 * around a FilePickerWidget.
 */
class FilePickerDialog : public QDialog
{
    Q_OBJECT

public:
    //! Constructor.
    explicit FilePickerDialog(QWidget *parent = nullptr);
    ~FilePickerDialog();

    //! Returns the selected URLs from the internal FilePickerWidget.
    QList<QUrl> getSelectedUrls();
    //! Sets the selected URLs in the internal FilePickerWidget.
    void setSelectedUrls(QList<QUrl> urls);
    //! Sets a single URL in the internal FilePickerWidget.
    void selectUrl(QUrl url);

private:
    Ui::FilePickerDialog _ui;
};

#endif // FILEPICKERDIALOG_H
