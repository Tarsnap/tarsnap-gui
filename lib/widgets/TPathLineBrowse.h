#ifndef TPATHLINEBROWSE_H
#define TPATHLINEBROWSE_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QObject>
#include <QString>
#include <QWidget>
WARNINGS_ENABLE

/* Forward declaration(s). */
namespace Ui
{
class TPathLineBrowse;
}
class QEvent;

/*!
 * \ingroup lib-widgets
 * \brief The TPathLineBrowse is a QWidget which consists of a QLabel, a
 * QLineEdit, and a QPushButton to trigger a QFileDialog to select a path.
 */
class TPathLineBrowse : public QWidget
{
    Q_OBJECT

    //! Text of the QLabel.
    Q_PROPERTY(QString label READ label WRITE setLabel)
    //! Placeholder text of the QLineEdit.
    Q_PROPERTY(
        QString placeholderText READ placeholderText WRITE setPlaceholderText)
    //! Title of the QFileDialog.
    Q_PROPERTY(QString dialogTitle READ dialogTitle WRITE setDialogTitle)

public:
    //! Constructor.
    explicit TPathLineBrowse(QWidget *parent = nullptr);
    ~TPathLineBrowse() override;

    //! Get the label text.
    QString label() const;
    //! Set the label text.
    void setLabel(const QString &text);

    //! Get the placeholder text of the QLineEdit.
    QString placeholderText() const;
    //! Set the placeholder text of the QLineEdit.
    void setPlaceholderText(const QString &text);

    //! Get the title of the QFileDialog.
    QString dialogTitle() const;
    //! Set the title of the QFileDialog.
    void setDialogTitle(const QString &text);

    //! Get the text of the QLineEdit.
    QString text() const;

    //! Get the status label text.
    QString statusText() const;
    //! Set the status label with an "ok".
    void setStatusOk(const QString &text);
    //! Set the status label with an error.
    void setStatusError(const QString &text);

public slots:
    //! Set the text of the QLineEdit.
    void setText(const QString &text);
    //! Clear the text of the QLineEdit.
    void clear();

signals:
    //! The text of the QLineEdit has changed.
    void textChanged(const QString &text);

protected:
    //! Handle translation change of language.
    void changeEvent(QEvent *event) override;

private slots:
    void browseClicked();

private:
    Ui::TPathLineBrowse *_ui;

    QString _dialogTitle;
};

#endif /* !TPATHLINEBROWSE_H */
