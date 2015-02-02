#ifndef TARSNAPCLI_H
#define TARSNAPCLI_H

#include <QThread>
#include <QProcess>
#include <QUuid>
#include <QRunnable>
#include <QVariant>

#define CMD_TARSNAP         "tarsnap"
#define CMD_TARSNAPKEYGEN   "tarsnap-keygen"

class TarsnapCLI : public QObject, public QRunnable
{
    Q_OBJECT

public:
    explicit TarsnapCLI(QUuid uuid = QUuid::createUuid());
    ~TarsnapCLI();

    void run();

    QString command() const;
    void setCommand(const QString &command);

    QStringList arguments() const;
    void setArguments(const QStringList &arguments);

    void killClient();
    QProcess::ProcessState statusClient();

    bool waitForClient();

    QString password() const;
    void setPassword(const QString &password);

    bool requiresPassword() const;
    void setRequiresPassword(bool requiresPassword);

    QUuid uuid() const;
    void setUuid(const QUuid &uuid);

    QVariant data() const;
    void setData(const QVariant &data);

signals:
    void clientFinished(QUuid uuid, QVariant data, int exitCode, QString output);
    void clientStarted(QUuid uuid);

private slots:
    void readProcessOutput();
    void processFinished();
    void processError();

private:
    QUuid            _uuid;
    QVariant         _data; // caller supplied data
    QProcess         *_process;
    QByteArray       _processOutput;
    QString          _command;
    QStringList      _arguments;
    QString          _password;
    bool             _requiresPassword;
};

#endif // TARSNAPCLI_H
