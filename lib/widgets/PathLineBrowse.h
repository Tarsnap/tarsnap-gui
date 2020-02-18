#ifndef PATHLINEBROWSE_H
#define PATHLINEBROWSE_H

#include <QWidget>

/* Forward declaration. */
namespace Ui
{
class PathLineBrowse;
}

/*!
 * \ingroup lib-widgets
 * \brief The PathLineBrowse is a QWidget which consists of a QLabel, a
 * QLineEdit, and a QPushButton to trigger a QFileDialog to select a path.
 */
class PathLineBrowse : public QWidget
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
    explicit PathLineBrowse(QWidget *parent = nullptr);
    ~PathLineBrowse();

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

public slots:
    //! Set the text of the QLineEdit.
    void setText(const QString &text);
    //! Clear the text of the QLineEdit.
    void clear();

signals:
    //! The text of the QLineEdit has changed.
    void textChanged(const QString &text);

private slots:
    void browseClicked();

private:
    Ui::PathLineBrowse *_ui;

    QString _dialogTitle;
};

#endif /* !PATHLINEBROWSE_H */
