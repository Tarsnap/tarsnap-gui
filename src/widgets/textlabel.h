#ifndef TEXTLABEL_H
#define TEXTLABEL_H

#include <QLabel>

class TextLabel : public QLabel
{
    Q_OBJECT

    Q_PROPERTY(Qt::TextElideMode elide READ elide WRITE setElide NOTIFY
                   elideChanged DESIGNABLE true)

public:
    explicit TextLabel(QWidget *parent = nullptr);
    ~TextLabel();

    Qt::TextElideMode elide() const;
    void setElide(const Qt::TextElideMode &elide);
    QString text(); // returns original text

    QSize sizeHint() const;

public slots:
    void setText(const QString &text);
    void clear();

protected:
    void resizeEvent(QResizeEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

signals:
    void elideChanged(Qt::TextElideMode elide);
    void clicked();

private:
    Qt::TextElideMode _elide;
    QString           _fullText;

    QString elideText(const QString &text);
};

#endif // TEXTLABEL_H
