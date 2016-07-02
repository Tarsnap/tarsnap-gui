#ifndef TESTCFSM_H
#define TESTCFSM_H

#include <QObject>
#include <QTextStream>
#include <QString>
#include <QHash>

#include "customfilesystemmodel.h"

class TestCFSM : public QObject
{
    Q_OBJECT

public:
    TestCFSM();
    ~TestCFSM();

public slots:
    // QFileSystemModel finished reading a directory from disk.
    void directoryLoaded(const QString &path);

    // CustomFileSystemModel changed some data;
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                     const QVector<int> &roles);

private:
    // The model we are testing.
    CustomFileSystemModel   _model;
    // The root directory of the model.
    QString                 _rootDir;
    // A hash table of emitted data from the model.
    QHash<QString, int>     _emittedHash;

    // Recursively checks for any unread subdirectories?
    bool needToReadSubdirs(const QString dirname);

    // Helper functions for processing the scenario text files.
    // "relname" is the "relative name", i.e. relative to the _rootDir.
    QString getRelname(const QString line);
    QModelIndex getIndex(const QString line);
    int getLineState(const QString line);
    int getCheckedStateInt(const QString line);

    // Main scenario-handling functions.
    int processActions(QTextStream &in);
    // Return 0 if success, 1 if a model error, 2 if an emit error.
    int processResults(QTextStream &in);
    int runScenario(const int num);
    void start();

    // For debugging and/or notifying of failed tests.
    void printModel();
    void printDir(const QString dirname, const int depth);
};

#endif
