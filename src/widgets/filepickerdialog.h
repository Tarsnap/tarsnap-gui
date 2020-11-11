#ifndef FILEPICKERDIALOG_H
#define FILEPICKERDIALOG_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QDialog>
#include <QList>
#include <QObject>
WARNINGS_ENABLE

/* Forward declaration(s). */
namespace Ui
{
class FilePickerDialog;
}
class QUrl;
class QWidget;

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
    ~FilePickerDialog() override;

    //! Returns the selected URLs from the internal FilePickerWidget.
    QList<QUrl> getSelectedUrls();
    //! Sets the selected URLs in the internal FilePickerWidget.
    void setSelectedUrls(const QList<QUrl> &urls);
    //! Sets a single URL in the internal FilePickerWidget.
    void selectUrl(const QUrl &url);

private:
    Ui::FilePickerDialog *_ui;
};

#endif // FILEPICKERDIALOG_H
