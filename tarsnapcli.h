#ifndef TARSNAPCLI_H
#define TARSNAPCLI_H

#include <QThread>
#include <QProcess>
#include <QUuid>

#define CMD_TARSNAP "tarsnap"
#define CMD_TARSNAPKEYGEN "tarsnap-keygen"

class TarsnapCLI : public QObject
{
    Q_OBJECT
public:
    explicit TarsnapCLI(QUuid uuid = QUuid::createUuid(), QObject *parent = 0);
    ~TarsnapCLI();

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

signals:
    void clientFinished(int exitCode, QString message, QString output);
    void clientStarted();

public slots:
    void runClient();

private slots:
    void readProcessOutput();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void processError(QProcess::ProcessError error);

private:
    QUuid            _uuid;
    QProcess         _process;
    QByteArray       _processOutput;
    QString          _command;
    QStringList      _arguments;
    QString          _password;
    bool             _requiresPassword;
    QThread          _thread;
};

#endif // TARSNAPCLI_H
