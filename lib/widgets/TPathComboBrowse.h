#ifndef TPATHCOMBOBROWSE_H
#define TPATHCOMBOBROWSE_H

#include "warnings-disable.h"

WARNINGS_DISABLE
#include <QObject>
#include <QString>
#include <QWidget>
WARNINGS_ENABLE

/* Forward declaration(s). */
namespace Ui
{
class TPathComboBrowse;
}
class QEvent;

/*!
 * \ingroup lib-widgets
 * \brief The TPathComboBrowse is a QWidget which consists of a QLabel, a
 * QComboBox, and a QPushButton to trigger a QFileDialog to select a path.
 */
class TPathComboBrowse : public QWidget
{
    Q_OBJECT

    //! Text of the QLabel.
    Q_PROPERTY(QString label READ label WRITE setLabel)
    //! Placeholder text for the QComboBox.
    Q_PROPERTY(
        QString placeholderText READ placeholderText WRITE setPlaceholderText)
    //! Title of the QFileDialog.
    Q_PROPERTY(QString dialogTitle READ dialogTitle WRITE setDialogTitle)
    //! Filter of the QFileDialog.
    Q_PROPERTY(QString dialogFilter READ dialogFilter WRITE setDialogFilter)

public:
    //! Constructor.
    explicit TPathComboBrowse(QWidget *parent = nullptr);
    ~TPathComboBrowse() override;

    //! Get the label text.
    QString label() const;
    //! Set the label text.
    void setLabel(const QString &text);

    //! Get the placeholder text of the QComboBox.
    QString placeholderText() const;
    //! Set the text of the QComboBox.
    void setPlaceholderText(const QString &text);

    //! Get the title of the QFileDialog.
    QString dialogTitle() const;
    //! Set the title of the QFileDialog.
    void setDialogTitle(const QString &text);

    //! Get the filter of the QFileDialog.
    QString dialogFilter() const;
    //! Set the filter of the QFileDialog.
    void setDialogFilter(const QString &text);

    //! Get the status label text.
    QString statusText() const;
    //! Set the status label with an "ok".
    void setStatusOk(const QString &text);
    //! Set the status label with an error.
    void setStatusError(const QString &text);

    //! Get the text of the QComboBox.
    QString text() const;

    //! Get the number of items in the QComboBox.
    int count() const;
    //! Add an item to the QComboBox.
    void addItem(const QString &text);

public slots:
    //! Set the text of the QComboBox.
    void setText(const QString &text);
    //! Clear the text of the QComboBox.
    void clear();

signals:
    //! The text of the QComboBox has changed.
    void textChanged(const QString &text);

protected:
    //! Handle translation change of language.
    void changeEvent(QEvent *event) override;

private slots:
    void browseClicked();

private:
    Ui::TPathComboBrowse *_ui;

    QString _dialogTitle;
    QString _dialogFilter;
};

#endif /* !TPATHCOMBOBROWSE_H */
